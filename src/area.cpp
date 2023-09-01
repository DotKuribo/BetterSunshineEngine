#include "area.hxx"
#include "module.hxx"
#include <JGadget/UnorderedMap.hxx>

// TODO: Initialize AreaInfo structs for each stage in base game.
// Custom registration should overwrite these defaults.

static JGadget::TUnorderedMap<u8, BetterSMS::Stage::AreaInfo> sAreaInfo(64);

BETTER_SMS_FOR_CALLBACK void initAreaInfo() {}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Stage::registerStageInfo(u8 stageID, AreaInfo *info) {
    if (sAreaInfo.find(stageID) != sAreaInfo.end()) {
        return false;
    }
    sAreaInfo[stageID] = *info;
    return true;
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Stage::deregisterStageInfo(u8 stageID) {
    sAreaInfo.erase(stageID);
}