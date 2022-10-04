#include <Dolphin/GX.h>
#include <Dolphin/OS.h>
#include <Dolphin/types.h>
#include <JSystem/JGeometry.hxx>
#include <JSystem/JUtility/JUTTexture.hxx>

#include "logging.hxx"

class SimpleTexAnimator {
public:
    SimpleTexAnimator(JUTTexture *textures, size_t texCount)
        : mTextures(textures), mTexCount(texCount) {}
    ~SimpleTexAnimator() {}

    void setCurrentFrame(u32 frame) { mCurrentFrame = static_cast<f32>(frame); }
    void setFrameRate(f32 rate) { mFrameRate = rate; }
    void setRotation(f32 rotation) { mRotation = rotation; }
    void setSpin(f32 degreesPerFrame) { mSpin = degreesPerFrame; };
    void setAlpha(u8 alpha) { mAlpha = alpha; }

    void draw(int x, int y, int w, int h) {
        if (mTexCount == 0)
            return;

        auto texture = mTextures[size_t(mCurrentFrame)];
        texture.load(GX_TEXMAP0);

        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetNumTexGens(1);
        GXLoadPosMtxImm(mInternalMtx, 0);

        setTevMode();

        GXBegin(0x80, GX_VTXFMT0, 4);
        GXPosition3s16(0, 0, 0);
        GXColor1u32(0xFFFFFFFF);
        wgPipe->S16 = 0;
        wgPipe->S16 = 0;
        wgPipe->S16 = w;
        wgPipe->S16 = 0;
        wgPipe->S16 = 0;
        GXColor1u32(0xFFFFFFFF);
        wgPipe->S16 = 0;
        wgPipe->S16 = 0;
        wgPipe->S16 = w;
        wgPipe->S16 = h;
        wgPipe->S16 = 0;
        wgPipe->S16 = 0;
        GXColor1u32(0xFFFFFFFF);
        wgPipe->S16 = 0;
        wgPipe->S16 = 0;
        wgPipe->S16 = 0;
        wgPipe->S16 = h;
        wgPipe->S16 = 0;
        wgPipe->S16 = 0;
        GXColor1u32(0xFFFFFFFF);
        wgPipe->S16 = 0;
        wgPipe->S16 = 0;

        Mtx identity;
        PSMTXIdentity(identity);

        GXLoadPosMtxImm(identity, 0);
        GXSetNumTexGens(0);
        GXSetNumTevStages(1);
        GXSetTevOp(GX_TEVSTAGE0, 4);
        GXSetTevOrder(0, 255, 255, 4);
        GXSetChanCtrl(4, 0, 0, 1, 0, GX_DF_NONE, GX_AF_NONE);
        GXSetVtxDesc(13, 0);
    }

private:
    void process() {
        f64 diff = OSTicksToSeconds(f64(OSGetTime())) - mLastTime;
        mCurrentFrame += diff * mFrameRate;
        mRotation += diff * mFrameRate * mSpin;

        makeMtx();
    }

    void makeMtx() {
        Mtx rotMtx;
        PSMTXRotRad(rotMtx, mRotation);
        PSMTXTrans(mInternalMtx, 500, 400, 0);
        PSMTXConcat(rotMtx, mInternalMtx, mInternalMtx);
    }

    void setTevMode() {
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 60, 0, 125);
        GXSetNumChans(1);
        GXSetChanCtrl(4, 1, 0, 1, 0, GX_DF_NONE, GX_AF_NONE);
        GXSetChanAmbColor(4, {0xFF, 0xFF, 0xFF, mAlpha});
        GXSetTevOrder(0, 0, 0, 255);
        GXSetTevColor(GX_TEVREG2, {0xFF, 0xFF, 0xFF, 0xFF});
        GXSetTevColorIn(GX_TEVSTAGE0, 8, 15, 15, 15);
        GXSetTevAlphaIn(GX_TEVSTAGE0, 4, 7, 7, 7);  // Possible make 4 be 3
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, 0, 0, 1, GX_TEVREG0);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, 0, 0, 1, GX_TEVREG0);
        GXSetTevKColor(0, {0xFF, 0xFF, 0xFF, 0xFF});
        GXSetTevKColor(2, {0xFF, 0xFF, 0xFF, 0xFF});
        GXSetTevKColorSel(GX_TEVSTAGE0, 0);
        GXSetTevKAlphaSel(GX_TEVSTAGE0, 2);
        GXSetTevColorIn(GX_TEVSTAGE0, 0, 8, 14, 15);
        GXSetTevAlphaIn(GX_TEVSTAGE0, 0, 4, 6, 7);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, 0, 0, 1, GX_TEVREG0);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, 0, 0, 1, GX_TEVREG0);
        GXSetNumTevStages(1);
        GXSetBlendMode(GX_BM_BLEND, 4, 5, 15);
    }

    JUTTexture *mTextures;
    size_t mTexCount;

private:
    Mtx mInternalMtx;
    f32 mRotation;
    f32 mSpin;

    f64 mLastTime;
    f32 mCurrentFrame;
    f32 mFrameRate;

    u8 mAlpha;
};