#include <SMS/macros.h>

#include "bmg.hxx"
#include "libs/container.hxx"

using namespace BetterSMS;

static TDictI<BMG::BMGCommandCallback> sBMGCommandCBs;

SMS_NO_INLINE bool BetterSMS::BMG::isBMGCommandRegistered(u8 identifier) {
    return sBMGCommandCBs.hasKey(identifier);
}

SMS_NO_INLINE bool BetterSMS::BMG::registerBMGCommandCallback(u8 identifier,
                                                              BMGCommandCallback cb) {
    if (sBMGCommandCBs.hasKey(identifier))
        return false;
    sBMGCommandCBs.set(identifier, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::BMG::deregisterBMGCommandCallback(u8 identifier) {
    if (!sBMGCommandCBs.hasKey(identifier))
        return false;
    sBMGCommandCBs.pop(identifier);
    return true;
}

static void formatCustomBMGCommands() {}