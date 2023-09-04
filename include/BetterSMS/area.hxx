#pragma once

#include <Dolphin/types.h>
#include <JSystem/JGadget/Vector.hxx>
#include <SMS/Manager/FlagManager.hxx>

#include "libs/cheathandler.hxx"
#include "libs/global_vector.hxx"

namespace BetterSMS {
    namespace Stage {
        struct AreaInfo {
            u8 mShineStageID;
            u8 mNormalStageID;
            TGlobalVector<s32> mScenarioIDs;
            TGlobalVector<s32> mExScenarioIDs;
            TGlobalVector<s32> mScenarioNameIDs;
            TGlobalVector<s32> mExScenarioNameIDs;
            u32 mShineSelectPaneID;
        };

        struct ExAreaInfo {
            u8 mParentStageID;
            s32 mShineID;
        };

        bool registerStageInfo(u8 stageID, AreaInfo *info);
        void deregisterStageInfo(u8 stageID);

        bool registerExStageInfo(u8 stageID, ExAreaInfo *info);
        void deregisterExStageInfo(u8 stageID);
    }  // namespace BMG
};     // namespace BetterSMS