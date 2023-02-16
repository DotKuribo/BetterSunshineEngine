#include <SMS/macros.h>

#include "module.hxx"

#include "libs/global_unordered_map.hxx"
#include "bmg.hxx"

using namespace BetterSMS;

static BetterSMS::TGlobalUnorderedMap<u8, BMG::BMGCommandCallback> sBMGCommandCBs(32);

BETTER_SMS_FOR_EXPORT bool BetterSMS::BMG::registerBMGCommandCallback(u8 identifier,
                                                              BMGCommandCallback cb) {
    if (sBMGCommandCBs.find(identifier) != sBMGCommandCBs.end())
        return false;
    sBMGCommandCBs[identifier] = cb;
    return true;
}

BETTER_SMS_FOR_EXPORT void BetterSMS::BMG::deregisterBMGCommandCallback(u8 identifier) {
    sBMGCommandCBs.erase(identifier);
}

static void formatCustomBMGCommands() {}