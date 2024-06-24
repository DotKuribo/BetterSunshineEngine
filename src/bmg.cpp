#include <SMS/macros.h>

#include "module.hxx"

#include "bmg.hxx"
#include "libs/global_vector.hxx"

using namespace BetterSMS;

struct BMGCommand {
  u8 mId;
  BMG::BMGCommandCallback mCallback;
};

static TGlobalVector<BMGCommand> sCustomBMGCommandCBs(32);

BETTER_SMS_FOR_EXPORT bool BetterSMS::BMG::registerBMGCommandCallback(u8 identifier,
                                                                      BMGCommandCallback cb) {
    for (const BMGCommand &command : sCustomBMGCommandCBs) {
        if (command.mId == identifier) {
            return false;
        }
    }
    sCustomBMGCommandCBs.push_back({identifier, cb});
    return true;
}

static void formatCustomBMGCommands() {}