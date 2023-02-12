#pragma once

#include <Dolphin/types.h>
#include <SMS/Manager/FlagManager.hxx>

#include "libs/cheathandler.hxx"

namespace BetterSMS {
    namespace BMG {
        typedef const char *(*BMGCommandCallback)(const u8 *flags, const u8 flagsLen,
                                                  TFlagManager *manager);

        bool registerBMGCommandCallback(u8 identifier, BMGCommandCallback cb);
        void deregisterBMGCommandCallback(u8 identifier);
    }
};  // namespace BetterSMS::BMG