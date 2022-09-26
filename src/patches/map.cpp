#include <Dolphin/string.h>

#include <SMS/collision/BGCheck.hxx>
#include <SMS/game/MarDirector.hxx>
#include <SMS/macros.h>
#include <SMS/manager/FlagManager.hxx>
#include <SMS/map/Map.hxx>
#include <SMS/mapobj/MapObjTree.hxx>

#include "common_sdk.h"
#include "module.hxx"

static bool isSeaBMDPresent(TMarDirector *director) {
    return JKRArchive::getGlbResource("/scene/map/map/sea.bmd") != nullptr;
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
