#pragma once

#include <Dolphin/types.h>
#include <JSystem/JGadget/Vector.hxx>
#include <SMS/Manager/FlagManager.hxx>
#include <SMS/System/MarDirector.hxx>

#include "libs/cheathandler.hxx"
#include "libs/global_vector.hxx"

#define BETTER_SMS_AREA_MAX   256
#define BETTER_SMS_EXAREA_MAX 256

namespace BetterSMS {
    namespace Stage {
         using NextStageCallback = void (*)(TMarDirector *);

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
            u8 mShineStageID;
            u8 mNormalStageID;
            TGlobalVector<s32> mShineIDs;
        };

        bool registerStageInfo(u8 stageID, AreaInfo *info);
        bool registerExStageInfo(u8 stageID, ExAreaInfo *info);
        void setNextStageHandler(NextStageCallback callback);

    }  // namespace Stage
};     // namespace BetterSMS