#include <SMS/macros.h>

#include "libs/global_unordered_map.hxx"
#include "bmg.hxx"

using namespace BetterSMS;

static BetterSMS::TGlobalUnorderedMap<u8, BMG::BMGCommandCallback> sBMGCommandCBs(64);

SMS_NO_INLINE bool BetterSMS::BMG::isBMGCommandRegistered(u8 identifier) {
    return sBMGCommandCBs.contains(identifier);
}

SMS_NO_INLINE bool BetterSMS::BMG::registerBMGCommandCallback(u8 identifier,
                                                              BMGCommandCallback cb) {
    if (isBMGCommandRegistered(identifier))
        return false;
    sBMGCommandCBs[identifier] = cb;
    return true;
}

SMS_NO_INLINE bool BetterSMS::BMG::deregisterBMGCommandCallback(u8 identifier) {
    if (!isBMGCommandRegistered(identifier))
        return false;
    sBMGCommandCBs.erase(identifier);
    return true;
}

static void formatCustomBMGCommands() {}