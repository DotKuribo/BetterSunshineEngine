#include <SMS/macros.h>

#include "libs/global_unordered_map.hxx"
#include "bmg.hxx"

using namespace BetterSMS;

static BetterSMS::TGlobalUnorderedMap<u8, BMG::BMGCommandCallback> sBMGCommandCBs(32);

SMS_NO_INLINE bool BetterSMS::BMG::registerBMGCommandCallback(u8 identifier,
                                                              BMGCommandCallback cb) {
    if (sBMGCommandCBs.find(identifier) != sBMGCommandCBs.end())
        return false;
    sBMGCommandCBs[identifier] = cb;
    return true;
}

SMS_NO_INLINE void BetterSMS::BMG::deregisterBMGCommandCallback(u8 identifier) {
    sBMGCommandCBs.erase(identifier);
}

static void formatCustomBMGCommands() {}