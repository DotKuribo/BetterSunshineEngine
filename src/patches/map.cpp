#include <Dolphin/string.h>

#include <SMS/Map/BGCheck.hxx>
#include <SMS/System/MarDirector.hxx>
#include <SMS/macros.h>
#include <SMS/Manager/FlagManager.hxx>
#include <SMS/Map/Map.hxx>
#include <SMS/MapObj/MapObjTree.hxx>
#include <SMS/raw_fn.hxx>

#include "module.hxx"

// TODO: Account for BetterSMS::areBugsPatched()

static bool isSeaBMDPresent(TMarDirector *director) {
    return JKRArchive::getGlbResource("/scene/Map/Map/sea.bmd") != nullptr;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8018A064, 0, 0, 0), isSeaBMDPresent);
SMS_WRITE_32(SMS_PORT_REGION(0x8018A068, 0, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x8018A06C, 0, 0, 0), 0x418201BC);
SMS_WRITE_32(SMS_PORT_REGION(0x8018A070, 0, 0, 0), 0x48000038);

static f32 considerDryGround(TMap *map, f32 x, f32 y, f32 z, const TBGCheckData **data) {
    return map->mCollisionData->checkGround(x, y, z, 1, data);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x800458F8, 0, 0, 0), considerDryGround);
SMS_PATCH_BL(SMS_PORT_REGION(0x80231878, 0, 0, 0),
             considerDryGround);  // Optimize shadow binding
SMS_WRITE_32(SMS_PORT_REGION(0x80231884, 0, 0, 0), 0x48000030);

static f32 patchedCheckGroundList(f32 x, f32 y, f32 z, u8 flags, const TBGCheckList *list,
                                  const TBGCheckData **out) {

    TBGCheckData *checkData;
    TBGCheckList *bgCheckNode;

    *out       = &TMapCollisionData::mIllegalCheckData;
    f32 exactY = -32767.0f;

    while (true) {
        do {
            if (!list)
                return exactY;

            checkData   = list->mColTriangle;
            bgCheckNode = list->mNextTriangle;
            list        = bgCheckNode;
        } while (y < checkData->mMinHeight);

        if ((flags & 4) != 0) {  // Pass through Check
            const u16 type = checkData->mType;
            if (type == 0x401 || type == 0x801 || type == 0x10A || type == 0x8400) {
                continue;
            }
        }

        if ((flags & 1) != 0) {  // Water Check
            const u16 type = checkData->mType;
            if (type == 0x100 || (type - 0x101) < 5 || type == 0x4104) {
                continue;
            }
        }

        const f32 ax = checkData->mVertices[0].x;
        const f32 az = checkData->mVertices[0].z;
        const f32 bz = checkData->mVertices[1].z;
        const f32 bx = checkData->mVertices[1].x;

        const bool abxCheck = -1.0f <= (az - z) * (bx - ax) - (ax - x) * (bz - az);

        if (!abxCheck)
            continue;

        const f32 cz = checkData->mVertices[2].z;
        const f32 cx = checkData->mVertices[2].x;

        const bool bcxCheck = -1.0f <= (bz - z) * (cx - bx) - (bx - x) * (cz - bz);
        const bool caxCheck = -1.0f <= (cz - z) * (ax - cx) - (cx - x) * (az - cz);

        if (!(bcxCheck && caxCheck))
            continue;

        const f32 sampleExactY =
            -(checkData->mProjectionFactor + x * checkData->mNormal.x + z * checkData->mNormal.z) /
            checkData->mNormal.y;
        if (y - (sampleExactY - 78.0f) < 0.0f)
            continue;

        if (sampleExactY > exactY) {
            *out   = checkData;
            exactY = sampleExactY;
        }
    }
}
SMS_PATCH_B(SMS_PORT_REGION(0x8018C334, 0, 0, 0), patchedCheckGroundList);

static bool clipActorsScaled(JDrama::TGraphics *graphics, const Vec *point, f32 radius) {
    TLiveActor *actor;
    SMS_FROM_GPR(31, actor);

    return ViewFrustumClipCheck__FPQ26JDrama9TGraphicsP3Vecf(
        graphics, point, radius * Max(Max(actor->mScale.x, actor->mScale.y), actor->mScale.z));
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8021B144, 0, 0, 0), clipActorsScaled);