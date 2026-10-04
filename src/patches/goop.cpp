#include <Dolphin/OS.h>
#include <Dolphin/mem.h>
#include <Dolphin/types.h>

#include <JSystem/JKernel/JKRHeap.hxx>
#include <JSystem/JUtility/JUTTexture.hxx>

#include <SMS/Map/PollutionLayer.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "module.hxx"

// Goop is drawn from its gameplay mask, a low resolution I8 texture stretched over the
// whole layer, so its edges show as stairs. Each layer gets a tent filtered copy of the
// mask that only the GPU reads. The mask itself is never written.

static_assert(sizeof(ResTIMG) == 0x20);

struct J3DTextureRes {
    u16 mNum;
    ResTIMG *mRes;
};

struct GoopLayer {
    u8 *mLayer;
    u8 *mMask;
    u8 *mDisplay;
    J3DTextureRes *mTexture;
    ResTIMG *mOrigRes;
    ResTIMG *mNewRes;
    u16 mWidth, mHeight;
    u16 mCursor;
    u16 mDirtyCount;
    u8 *mDirty;    // Passes left to watch, per 8x4 texel tile
    u32 *mHashes;  // Mask hash per tile when its display was last computed
    u8 mIndex;
    u8 mNonce;
    u8 mPass;
    bool mSoft;
};

// TPollutionLayer fields
#define LAYER_INDEX(l)     (*reinterpret_cast<u32 *>((l) + 0x04))
#define LAYER_MODELDATA(l) (*reinterpret_cast<u8 **>((l) + 0x24))
#define LAYER_MODEL(l)     (*reinterpret_cast<u8 **>((l) + 0x28))
#define LAYER_BOUNDS(l)    (reinterpret_cast<f32 *>((l) + 0x38))  // x0, x1, z0, z1
#define LAYER_MASK(l)      (*reinterpret_cast<u8 **>((l) + 0x54))
#define LAYER_MASKRES(l)   (*reinterpret_cast<ResTIMG **>((l) + 0x58))

static constexpr u32 MaxGoopLayers = 16;
static GoopLayer *sGoopLayers[MaxGoopLayers];

static inline u32 rowOffset(u32 y, u32 tilesW) { return (y >> 2) * tilesW * 32 + ((y & 3) << 3); }
static inline u32 colOffset(u32 x) { return ((x >> 3) << 5) + (x & 7); }

static void smoothRows(GoopLayer *g, u32 y0, u32 y1, u32 xa, u32 xb) {
    u16 rowSum[520];
    const u32 w = g->mWidth, h = g->mHeight, tilesW = w >> 3;
    const u8 *src = g->mMask;
    u8 *dst       = g->mDisplay;
    const u32 xl  = xa ? xa - 1 : 0;
    const u32 xr  = xb + 1 < w ? xb + 1 : w - 1;

    for (u32 y = y0; y <= y1; ++y) {
        const u32 up   = rowOffset(y ? y - 1 : 0, tilesW);
        const u32 mid  = rowOffset(y, tilesW);
        const u32 down = rowOffset(y + 1 < h ? y + 1 : h - 1, tilesW);
        for (u32 x = xl; x <= xr; ++x) {
            const u32 o = colOffset(x);
            rowSum[x]   = src[up + o] + 2 * src[mid + o] + src[down + o];
        }
        for (u32 x = xa; x <= xb; ++x) {
            const u32 left          = x ? rowSum[x - 1] : rowSum[x];
            const u32 right         = x + 1 < w ? rowSum[x + 1] : rowSum[x];
            dst[mid + colOffset(x)] = (left + 2 * rowSum[x] + right + 8) >> 4;
        }
    }

    for (u32 ty = y0 >> 2; ty <= y1 >> 2; ++ty)
        DCStoreRange(dst + (ty * tilesW + (xa >> 3)) * 32, ((xb >> 3) - (xa >> 3) + 1) * 32);
}

// The mask is copied back from the GPU a few frames after a stamp, so stamped tiles are
// watched for 8 passes.
static void markTiles(GoopLayer *g, s32 cx, s32 cy, s32 radius) {
    s32 xa = cx - radius, xb = cx + radius, ya = cy - radius, yb = cy + radius;
    if (xa < 0)
        xa = 0;
    if (ya < 0)
        ya = 0;
    if (xb >= g->mWidth)
        xb = g->mWidth - 1;
    if (yb >= g->mHeight)
        yb = g->mHeight - 1;
    if (xa > xb || ya > yb)
        return;

    const u32 tilesW = g->mWidth >> 3;
    for (u32 ty = ya >> 2; ty <= u32(yb) >> 2; ++ty) {
        u8 *row = g->mDirty + ty * tilesW;
        for (u32 tx = xa >> 3; tx <= u32(xb) >> 3; ++tx) {
            if (row[tx] == 0)
                g->mDirtyCount += 1;
            row[tx] = 8;
        }
    }
}

static u32 hashTile(const GoopLayer *g, u32 tile) {
    const u32 *data = reinterpret_cast<const u32 *>(g->mMask + tile * 32);
    u32 hash        = 0;
    for (u32 i = 0; i < 8; ++i)
        hash = (hash << 5 | hash >> 27) ^ data[i];
    return hash;
}

// A watched tile is only recomputed, with a one texel border, once its mask has changed
static void updateDirtyTiles(GoopLayer *g) {
    const u32 w = g->mWidth, h = g->mHeight, tilesW = w >> 3, tilesH = h >> 2;
    for (u32 ty = 0; ty < tilesH && g->mDirtyCount; ++ty) {
        u8 *row     = g->mDirty + ty * tilesW;
        u32 *hashes = g->mHashes + ty * tilesW;
        s32 start   = -1;
        for (u32 tx = 0; tx <= tilesW; ++tx) {
            bool changed = false;
            if (tx < tilesW && row[tx]) {
                if (--row[tx] == 0)
                    g->mDirtyCount -= 1;
                const u32 hash = hashTile(g, ty * tilesW + tx);
                changed        = hash != hashes[tx];
                hashes[tx]     = hash;
            }
            if (changed && start < 0) {
                start = tx;
            } else if (!changed && start >= 0) {
                const u32 y0 = ty * 4, x0 = start * 8, x1 = tx * 8 - 1;
                smoothRows(g, y0 ? y0 - 1 : 0, y0 + 4 < h ? y0 + 4 : h - 1, x0 ? x0 - 1 : 0,
                           x1 + 1 < w ? x1 + 1 : w - 1);
                start = -1;
            }
        }
    }
}

// Soft edge for the brown goop material only: remap alpha around the 0x80 threshold
// and blend instead of the hard alpha test.
#define VT_TEVBLOCK4    0x803E0AB0
#define VT_PEBLOCK_FULL 0x803E0968

static constexpr u32 TevStage1Hard = 0xC331FF80, TevStage1Soft = 0xC322FF80;
static constexpr u32 TevStage2Hard = 0xC500FF80, TevStage2Soft = 0xC509FF80;
static constexpr u32 BlendHard = 0x00010003, BlendSoft = 0x01040503;

// The TEV stage words are unaligned, which the Gekko handles in hardware.
static inline u32 &bpWord(u8 *p) { return *reinterpret_cast<u32 *>(p); }

static void setSoftEdge(u8 *layer, bool soft) {
    u8 *modelData  = LAYER_MODELDATA(layer);
    const u16 num  = *reinterpret_cast<u16 *>(modelData + 0x24);
    u8 **materials = *reinterpret_cast<u8 ***>(modelData + 0x28);

    const u32 from1 = soft ? TevStage1Hard : TevStage1Soft,
              to1   = soft ? TevStage1Soft : TevStage1Hard;
    const u32 from2 = soft ? TevStage2Hard : TevStage2Soft,
              to2   = soft ? TevStage2Soft : TevStage2Hard;
    const u32 fromB = soft ? BlendHard : BlendSoft, toB = soft ? BlendSoft : BlendHard;

    for (u32 i = 0; i < num; ++i) {
        u8 *tev = *reinterpret_cast<u8 **>(materials[i] + 0x28);
        u8 *pe  = *reinterpret_cast<u8 **>(materials[i] + 0x30);
        if (*reinterpret_cast<u32 *>(tev) != VT_TEVBLOCK4 ||
            *reinterpret_cast<u32 *>(pe) != VT_PEBLOCK_FULL)
            continue;
        if (bpWord(tev + 0x29) != from1 || bpWord(tev + 0x31) != from2 || bpWord(pe + 0xC) != fromB)
            continue;
        bpWord(tev + 0x29) = to1;
        bpWord(tev + 0x31) = to2;
        bpWord(pe + 0xC)   = toB;
        pe[0xA]            = soft ? 1 : 128;
    }
}

// Large ground layers lock their material packet, so their display list keeps the mask
// address in its BP 0x94 (TX_IMAGE3) command instead of reading the J3DTexture.
static void patchLockedDisplayList(GoopLayer *g, u8 *image) {
    u8 *packet = *reinterpret_cast<u8 **>(LAYER_MODEL(g->mLayer) + 0x80);
    if (!(*reinterpret_cast<u32 *>(packet + 0x10) & 1))
        return;

    const u32 maskCmd    = 0x94000000 | ((reinterpret_cast<u32>(g->mMask) & 0x1FFFFFFF) >> 5);
    const u32 displayCmd = 0x94000000 | ((reinterpret_cast<u32>(g->mDisplay) & 0x1FFFFFFF) >> 5);
    const u32 newCmd     = 0x94000000 | ((reinterpret_cast<u32>(image) & 0x1FFFFFFF) >> 5);

    u8 **buffers = *reinterpret_cast<u8 ***>(packet + 0x30);
    for (u32 i = 0; i < 2; ++i) {
        u8 *cmd = buffers[i] + 5;
        if (cmd[0] != 0x61)
            continue;
        if (bpWord(cmd + 1) == maskCmd || bpWord(cmd + 1) == displayCmd) {
            bpWord(cmd + 1) = newCmd;
            DCStoreRange(cmd, 8);
        }
    }
}

static bool isLayerVTable(u32 vt) {
    return vt == 0x803C2160 || vt == 0x803C21BC || vt == 0x803C2218 || vt == 0x803C1E90 ||
           vt == 0x803C2274 || vt == 0x803C1EEC;
}

void resetGoopLayers(TApplication *app) {
    for (u32 i = 0; i < MaxGoopLayers; ++i)
        sGoopLayers[i] = nullptr;
}

static GoopLayer *findGoopLayer(u8 *layer) {
    for (u32 i = 0; i < MaxGoopLayers; ++i) {
        if (sGoopLayers[i] && sGoopLayers[i]->mLayer == layer)
            return sGoopLayers[i];
    }
    return nullptr;
}

static GoopLayer *findGoopLayerByIndex(u32 index) {
    for (u32 i = 0; i < MaxGoopLayers; ++i) {
        if (sGoopLayers[i] && sGoopLayers[i]->mIndex == (index & 0xFF))
            return sGoopLayers[i];
    }
    return nullptr;
}

static void initGoopLayer(u8 *layer, const char *name) {
    initTexImage__15TPollutionLayerFPCc(layer, name);

    const u32 index = LAYER_INDEX(layer);
    if (index == 0)
        resetGoopLayers(nullptr);

    GoopLayer **slot = nullptr;
    for (u32 i = 0; i < MaxGoopLayers; ++i) {
        if (sGoopLayers[i] && sGoopLayers[i]->mIndex == index)
            sGoopLayers[i] = nullptr;
        if (!sGoopLayers[i] && !slot)
            slot = &sGoopLayers[i];
    }
    if (!slot || !isLayerVTable(*reinterpret_cast<u32 *>(layer)))
        return;

    auto *texture = *reinterpret_cast<J3DTextureRes **>(LAYER_MODELDATA(layer) + 0xAC);
    u8 *mask      = LAYER_MASK(layer);
    ResTIMG *res  = LAYER_MASKRES(layer);
    const u32 w = res->mWidth, h = res->mHeight, num = texture->mNum;
    if (res->mFormat != ResTIMG::I8 || w > 512 || h > 1024 || (w & 7) || (h & 3) || num == 0 ||
        num > 16)
        return;

    const u32 headerSize = (sizeof(GoopLayer) + 31 & ~31) + num * sizeof(ResTIMG);
    const u32 tiles      = (w >> 3) * (h >> 2);
    const u32 size       = headerSize + w * h + tiles * 5;
    JKRHeap *heap        = JKRHeap::sCurrentHeap;
    if (heap->getFreeSize() < size + 0x80000)  // Leave 512 KB to the stage
        return;

    u8 *block = static_cast<u8 *>(heap->alloc(size, 32));
    if (!block)
        return;

    auto *g       = reinterpret_cast<GoopLayer *>(block);
    auto *newRes  = reinterpret_cast<ResTIMG *>(block + (sizeof(GoopLayer) + 31 & ~31));
    u8 *display   = block + headerSize;
    ResTIMG *orig = texture->mRes;

    // ResTIMG data offsets are relative to each header, so rebase them for the copy
    for (u32 i = 0; i < num; ++i) {
        newRes[i]                = orig[i];
        u8 *origBase             = reinterpret_cast<u8 *>(&orig[i]);
        u8 *newBase              = reinterpret_cast<u8 *>(&newRes[i]);
        u8 *image                = origBase + orig[i].mTextureOffset;
        newRes[i].mTextureOffset = (image == mask ? display : image) - newBase;
        if (orig[i].mPaletteColors)
            newRes[i].mPaletteOffset = origBase + orig[i].mPaletteOffset - newBase;
    }

    *g          = GoopLayer{};
    g->mLayer   = layer;
    g->mMask    = mask;
    g->mDisplay = display;
    g->mTexture = texture;
    g->mOrigRes = orig;
    g->mNewRes  = newRes;
    g->mWidth   = w;
    g->mHeight  = h;
    g->mIndex   = index;
    g->mHashes  = reinterpret_cast<u32 *>(display + w * h);
    g->mDirty   = display + w * h + tiles * 4;
    memset(g->mDirty, 0, tiles);
    for (u32 i = 0; i < tiles; ++i)
        g->mHashes[i] = hashTile(g, i);
    *slot = g;

    smoothRows(g, 0, h - 1, 0, w - 1);
    DCStoreRange(block, size);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801A0EB8, 0, 0, 0), initGoopLayer);

static void updateGoopLayer(TPollutionLayer *pollution, u32 flags, JDrama::TGraphics *graphics) {
    pollution->TJointModel::perform(flags, graphics);
    if (!(flags & 1))
        return;

    u8 *layer    = reinterpret_cast<u8 *>(pollution);
    GoopLayer *g = findGoopLayer(layer);
    if (!g || LAYER_MASK(layer) != g->mMask)
        return;
    if (g->mTexture->mRes != g->mOrigRes && g->mTexture->mRes != g->mNewRes)
        return;

    g->mTexture->mRes = g->mNewRes;
    if ((g->mPass++ & 15) == 0)
        patchLockedDisplayList(g, g->mDisplay);
    if (!g->mSoft) {
        setSoftEdge(layer, true);
        g->mSoft = true;
    }

    updateDirtyTiles(g);

    // Background refresh for any writer we don't track
    const u32 y0 = g->mCursor;
    const u32 y1 = y0 + 3 < g->mHeight ? y0 + 3 : g->mHeight - 1;
    smoothRows(g, y0, y1, 0, g->mWidth - 1);
    g->mCursor = y1 + 1 >= g->mHeight ? 0 : y1 + 1;

    // Dolphin samples texture data to detect CPU writes, texel (0, 0) included. Changing
    // its low bits on every pass keeps the cached texture from going stale.
    g->mNonce      = g->mNonce >= 2 ? 0 : g->mNonce + 1;
    g->mDisplay[0] = (g->mDisplay[0] & 0xFC) | g->mNonce;
    DCStoreRange(g->mDisplay, 32);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801A12C8, 0, 0, 0), updateGoopLayer);

static SMS_ASM_FUNC void pushTask_(void *stamp, u32 index, u32 size, u32 x, u32 y, u32 value) {
    SMS_ASM_BLOCK("lwz 9, 0x8 (3)                                   \n\t"
                  "b pushTask__18TPollutionTexStampFUcUsUsUss + 4 \n\t");
}

static void pushTask(void *stamp, u32 index, u32 size, u32 x, u32 y, u32 value) {
    if (GoopLayer *g = findGoopLayerByIndex(index))
        markTiles(g, x & 0xFFFF, y & 0xFFFF, (size & 0xFFFF) + 2);
    pushTask_(stamp, index, size, x, y, value);
}
SMS_PATCH_B(SMS_PORT_REGION(0x8019ABAC, 0, 0, 0), pushTask);

static SMS_ASM_FUNC void pushModelStampTask_(void *layer, u32 index, J3DModel *model) {
    SMS_ASM_BLOCK("lhz 0, 0x28 (3)                                              \n\t"
                  "b pushModelStampTask__22TPollutionCounterLayerFUcP8J3DModel + 4 \n\t");
}

// Model stamps (Petey's puddles, Gooper Blooper, Shadow Mario) spread over several
// frames around the stamp model, about 400 units times its scale.
static void pushModelStampTask(void *counter, u32 index, J3DModel *model) {
    if (GoopLayer *g = findGoopLayerByIndex(index)) {
        const f32 *bounds = LAYER_BOUNDS(g->mLayer);
        const f32 *mtx    = reinterpret_cast<f32 *>(model);
        const s32 x0 = bounds[0], dx = s32(bounds[1]) - x0;
        const s32 z0 = bounds[2], dz = s32(bounds[3]) - z0;
        if (dx > 0 && dz > 0) {
            const s32 radius = s32(mtx[0x14 / 4] * 400.0f) * g->mWidth / dx + 2;
            markTiles(g, (s32(mtx[0x2C / 4]) - x0) * g->mWidth / dx,
                      (s32(mtx[0x4C / 4]) - z0) * g->mHeight / dz, radius);
        }
    }
    pushModelStampTask_(counter, index, model);
}
SMS_PATCH_B(SMS_PORT_REGION(0x8019B120, 0, 0, 0), pushModelStampTask);
