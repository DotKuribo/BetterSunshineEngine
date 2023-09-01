#pragma once

#include <Dolphin/types.h>
#include <JSystem/JGadget/Vector.hxx>
#include <SMS/Manager/FlagManager.hxx>

#include "libs/cheathandler.hxx"

namespace BetterSMS {
    namespace Stage {
        struct AreaInfo {
            //u8 mShineStageID;
            JGadget::TVector<u8> mScenarioIDs;
            JGadget::TVector<u8> mExScenarioIDs;
        };

        bool registerStageInfo(u8 stageID, AreaInfo *info);
        void deregisterStageInfo(u8 stageID);
    }  // namespace BMG
};     // namespace BetterSMS