#include <Dolphin/string.h>

#include <SMS/game/MarDirector.hxx>
#include <SMS/macros.h>
#include <SMS/manager/FlagManager.hxx>
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