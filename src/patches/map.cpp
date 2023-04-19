#include <Dolphin/string.h>

#include <SMS/Map/BGCheck.hxx>
#include <SMS/System/MarDirector.hxx>
#include <SMS/macros.h>
#include <SMS/Manager/FlagManager.hxx>
#include <SMS/Map/Map.hxx>
#include <SMS/MapObj/MapObjTree.hxx>
#include <SMS/raw_fn.hxx>

#include "module.hxx"
#include "p_settings.hxx"

// TODO: Account for BetterSMS::areBugsPatched()

static bool isSeaBMDPresent(TMarDirector *director) {
    return JKRArchive::getGlbResource("/scene/Map/Map/sea.bmd") != nullptr && JKRArchive::getGlbResource("/scene/Map/Map/seaindirect.bmd") != nullptr;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8018A064, 0, 0, 0), isSeaBMDPresent);
SMS_WRITE_32(SMS_PORT_REGION(0x8018A068, 0, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x8018A06C, 0, 0, 0), 0x418201BC);
SMS_WRITE_32(SMS_PORT_REGION(0x8018A070, 0, 0, 0), 0x48000038);

static f32 considerDryGround(TMap *map, f32 x, f32 y, f32 z, const TBGCheckData **data) {
    return map->mCollisionData->checkGround(x, y, z, 5, data);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x800458F8, 0, 0, 0), considerDryGround);

static f32 considerShadowGround(TMap *map, f32 x, f32 y, f32 z, const TBGCheckData **data) {
    return map->mCollisionData->checkGround(x, y, z, 4, data);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80231878, 0, 0, 0),
             considerShadowGround);  // Optimize shadow binding
SMS_WRITE_32(SMS_PORT_REGION(0x80231884, 0, 0, 0), 0x48000030);

static bool clipActorsScaled(JDrama::TGraphics *graphics, const Vec *point, f32 radius) {
    TLiveActor *actor;
    SMS_FROM_GPR(31, actor);

    return ViewFrustumClipCheck__FPQ26JDrama9TGraphicsP3Vecf(
        graphics, point, radius * Max(Max(actor->mScale.x, actor->mScale.y), actor->mScale.z));
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8021B144, 0, 0, 0), clipActorsScaled);
