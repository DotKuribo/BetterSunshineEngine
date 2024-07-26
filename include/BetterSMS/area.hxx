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

        class ShineAreaInfo {
        public:
            ShineAreaInfo() = delete;
            ShineAreaInfo(u8 shineStageID) : mShineStageID(shineStageID) {}
            ShineAreaInfo(u8 shineStageID, u32 shineSelectPaneID)
                : mShineStageID(shineStageID), mShineSelectPaneID(shineSelectPaneID) {}

            [[nodiscard]] u8 getShineStageID() const { return mShineStageID; }
            [[nodiscard]] u32 getShineSelectPaneID() const { return mShineSelectPaneID; }

            [[nodiscard]] const TGlobalVector<s32> &getScenarioIDs() const { return mScenarioIDs; }
            [[nodiscard]] const TGlobalVector<s32> &getExScenarioIDs() const {
                return mExScenarioIDs;
            }
            [[nodiscard]] const TGlobalVector<s32> &getScenarioNameIDs() const {
                return mScenarioNameIDs;
            }
            [[nodiscard]] const TGlobalVector<s32> &getExScenarioNameIDs() const {
                return mExScenarioNameIDs;
            }

            void addScenario(s32 scenarioID, s32 scenarioNameID) {
                if (scenarioID != -1) {
                    for (const s32 &id : mScenarioIDs) {
                        if (id == scenarioID) {
                            return;
                        }
                    }
                }
                mScenarioIDs.push_back(scenarioID);
                mScenarioNameIDs.push_back(scenarioNameID);
            }

            void addExScenario(s32 exScenarioID, s32 exScenarioNameID) {
                if (exScenarioID != -1) {
                    for (const s32 &id : mScenarioIDs) {
                        if (id == exScenarioID) {
                            return;
                        }
                    }
                }
                mExScenarioIDs.push_back(exScenarioID);
                mExScenarioNameIDs.push_back(exScenarioNameID);
            }

        private:
            u8 mShineStageID;
            TGlobalVector<s32> mScenarioIDs       = {};
            TGlobalVector<s32> mExScenarioIDs     = {};
            TGlobalVector<s32> mScenarioNameIDs   = {};
            TGlobalVector<s32> mExScenarioNameIDs = {};
            u32 mShineSelectPaneID                = '\0';
        };

        bool registerShineStage(ShineAreaInfo *info);
        bool registerNormalStage(u8 normalStageID, u8 shineStageID);
        bool registerExStage(u8 exStageID, u8 shineStageID, s32 shineID);

        void setNextStageHandler(NextStageCallback callback);

    }  // namespace Stage
};     // namespace BetterSMS