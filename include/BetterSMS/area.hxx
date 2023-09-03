#pragma once

#include <Dolphin/types.h>
#include <JSystem/JGadget/Vector.hxx>
#include <SMS/Manager/FlagManager.hxx>

#include "libs/cheathandler.hxx"

namespace BetterSMS {
    namespace Stage {
        struct AreaInfo {
            u8 mShineStageID;
            u8 mNormalStageID;
            JGadget::TVector<u8> mScenarioIDs;
            JGadget::TVector<u8> mExScenarioIDs;
            JGadget::TVector<u8> mScenarioNameIDs;
            JGadget::TVector<u8> mExScenarioNameIDs;
            u32 mShineSelectPaneID;
        };

        struct ExAreaInfo {
            u8 mParentStageID;
            u8 mShineID;
        };

        bool registerStageInfo(u8 stageID, AreaInfo *info);
        void deregisterStageInfo(u8 stageID);

        bool registerExStageInfo(u8 stageID, ExAreaInfo *info);
        void deregisterExStageInfo(u8 stageID);
    }  // namespace BMG
};     // namespace BetterSMS