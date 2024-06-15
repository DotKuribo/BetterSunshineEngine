#include "area.hxx"
#include "module.hxx"
#include <JGadget/UnorderedMap.hxx>
#include <SMS/GC2D/SelectMenu.hxx>
#include <SMS/Manager/FlagManager.hxx>
#include <SMS/raw_fn.hxx>

static void moveStage_override(TMarDirector *director);

namespace BetterSMS {

    namespace Stage {

        static AreaInfo *sAreaInfos[BETTER_SMS_AREA_MAX];
        static ExAreaInfo *sExAreaInfos[BETTER_SMS_EXAREA_MAX];

        AreaInfo **getAreaInfos() { return sAreaInfos; }
        ExAreaInfo **getExAreaInfos() { return sExAreaInfos; }

        static NextStageCallback sNextStageHandler = moveStage_override;

    }  // namespace Stage

}  // namespace BetterSMS

using namespace BetterSMS::Stage;

static size_t getScenariosForScene(int sceneID) {
    if (sceneID == 0 || sceneID == 1)
        return 0;

    return 8;
}

static size_t getExScenariosForScene(int sceneID) { return 4; }

BETTER_SMS_FOR_CALLBACK void initAreaInfo() {
    const u8 **baseGameShineTable =
        reinterpret_cast<const u8 **>(SMS_PORT_REGION(0x803C0CC8, 0, 0, 0));
    const u8 **baseGameExShineTable =
        reinterpret_cast<const u8 **>(SMS_PORT_REGION(0x803C0CF0, 0, 0, 0));
    const s32 *baseGameScenarioNameTable =
        reinterpret_cast<const s32 *>(SMS_PORT_REGION(0x803C0D18, 0, 0, 0));
    const s32 *baseGameNormalStageTable =
        reinterpret_cast<const s32 *>(SMS_PORT_REGION(0x803C0E30, 0, 0, 0));
    const u8 *baseGameStageTable =
        reinterpret_cast<const u8 *>(SMS_PORT_REGION(0x803DF498, 0, 0, 0));
    const u8 *baseGameExShineTable2 =
        reinterpret_cast<const u8 *>(SMS_PORT_REGION(0x803DF4D8, 0, 0, 0));
    {
        auto *oldHeap = JKRHeap::sRootHeap->becomeCurrentHeap();

        const u32 scenePaneIDs[] = {0,      0, 'bi_0', 'rc_0', 'mm_0', 'pi_0',
                                    'sr_0', 0, 'mo_0', 'mr_0', 0};

        for (int i = 0; i < 64; ++i) {
            auto info                = new BetterSMS::Stage::AreaInfo;
            info->mShineSelectPaneID = scenePaneIDs[i];
            info->mNormalStageID     = i;
            info->mShineStageID      = baseGameStageTable[i];
            if (i < 10 && baseGameShineTable[info->mShineStageID]) {
                for (int j = 0; j < getScenariosForScene(i); ++j) {
                    auto scenarioID = baseGameShineTable[info->mShineStageID][j];
                    info->mScenarioIDs.push_back(scenarioID);
                    info->mScenarioNameIDs.push_back(baseGameScenarioNameTable[scenarioID]);
                }
            }
            if (i < 10 && baseGameExShineTable[info->mShineStageID]) {
                for (int j = 0; j < getExScenariosForScene(i); ++j) {
                    auto scenarioID = baseGameExShineTable[info->mShineStageID][j];
                    info->mExScenarioIDs.push_back(scenarioID);
                    info->mExScenarioNameIDs.push_back(baseGameScenarioNameTable[scenarioID]);
                }
            }
            BetterSMS::Stage::registerStageInfo(i, info);
        }

        for (int i = 0; i < 32; ++i) {
            auto exInfo            = new BetterSMS::Stage::ExAreaInfo;
            exInfo->mNormalStageID = i + 0x14;
            exInfo->mShineStageID  = baseGameStageTable[i + 0x14];
            exInfo->mShineID = baseGameExShineTable2[i] != 0xFF ? baseGameExShineTable2[i] : -1;
            BetterSMS::Stage::registerExStageInfo(exInfo->mNormalStageID, exInfo);
        }

        oldHeap->becomeCurrentHeap();
    }
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Stage::registerStageInfo(u8 stageID, AreaInfo *info) {
    if (sAreaInfos[stageID]) {
        OSReport("[WARN] Overwriting stage info for stage %d\n", stageID);
    }
    sAreaInfos[stageID] = info;
    return true;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Stage::registerExStageInfo(u8 stageID, ExAreaInfo *info) {
    if (sExAreaInfos[stageID]) {
        OSReport("[WARN] Overwriting ex stage info for stage %d\n", stageID);
    }
    sExAreaInfos[stageID] = info;
    return true;
}

void BetterSMS::Stage::setNextStageHandler(NextStageCallback callback) {
    sNextStageHandler = callback;
}

static void moveStageHandler(TMarDirector *director) { sNextStageHandler(director); }
SMS_PATCH_B(SMS_PORT_REGION(0x80297584, 0, 0, 0), moveStageHandler);

static s32 SMS_getShineID(u32 stageID, u32 scenarioID, bool isExStage) {
    if (!sAreaInfos[stageID]) {
        return -1;
    }
    const BetterSMS::Stage::AreaInfo &info = *sAreaInfos[stageID];
    if (isExStage) {
        if (scenarioID >= info.mExScenarioIDs.size()) {
            return -1;
        }
        return info.mExScenarioIDs[scenarioID];
    } else {
        if (scenarioID >= info.mScenarioIDs.size()) {
            return -1;
        }
        return info.mScenarioIDs[scenarioID];
    }
}
SMS_PATCH_B(SMS_PORT_REGION(0x8016FAC0, 0, 0, 0), SMS_getShineID);
SMS_PATCH_B(SMS_PORT_REGION(0x80175AF8, 0, 0, 0), SMS_getShineID);
SMS_PATCH_B(SMS_PORT_REGION(0x8017CC6C, 0, 0, 0), SMS_getShineID);

static s32 SMS_getShineIDofExStage(u32 exStageID) {
    if (!sExAreaInfos[exStageID])
        return -1;
    return sExAreaInfos[exStageID]->mShineID;
}
SMS_PATCH_B(SMS_PORT_REGION(0x802a8a98, 0, 0, 0), SMS_getShineIDofExStage);

static s32 SMS_getShineStage(u32 stageID) {
    if (!sAreaInfos[stageID])
        return -1;
    return sAreaInfos[stageID]->mShineStageID;
}
SMS_PATCH_B(SMS_PORT_REGION(0x802A8AC8, 0, 0, 0), SMS_getShineStage);

static TExPane *constructExPaneForSelectScreen(TExPane *pane, J2DScreen *screen) {
    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    // This check is only necessary once
    SMS_ASSERT(sAreaInfos[menu->mAreaID]->mShineSelectPaneID != 0,
               "Tried to open shine select screen for an area that has no pane ID!");

    return (TExPane *)__ct__7TExPaneFP9J2DScreenUl(pane, screen,
                                                   sAreaInfos[menu->mAreaID]->mShineSelectPaneID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80174D40, 0, 0, 0), constructExPaneForSelectScreen);

static TBoundPane *constructBoundPaneForSelectScreenA(TBoundPane *pane, J2DScreen *screen) {
    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    u32 paneID = (sAreaInfos[menu->mAreaID]->mShineSelectPaneID & 0xFFFFFF00) | 'a';

    return (TBoundPane *)__ct__10TBoundPaneFP9J2DScreenUl(pane, screen, paneID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80174D88, 0, 0, 0), constructBoundPaneForSelectScreenA);

static TBoundPane *constructBoundPaneForSelectScreenB(TBoundPane *pane, J2DScreen *screen) {
    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    u32 paneID = (sAreaInfos[menu->mAreaID]->mShineSelectPaneID & 0xFFFFFF00) | 'b';

    return (TBoundPane *)__ct__10TBoundPaneFP9J2DScreenUl(pane, screen, paneID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80174DD0, 0, 0, 0), constructBoundPaneForSelectScreenB);

static bool getShineFlagForSelectScreen1() {
    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    int shineID = SMS_getShineID(SMS_getShineStage(menu->mAreaID), 0, false);
    if (shineID == -1) {
        return false;
    }

    return TFlagManager::smInstance->getShineFlag(shineID);
}
SMS_WRITE_32(SMS_PORT_REGION(0x80174B40, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80174B44, 0, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x80174B48, 0, 0, 0), getShineFlagForSelectScreen1);

static bool getShineFlagForSelectScreen2() {
    int episodeID;
    SMS_FROM_GPR(26, episodeID);

    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    int shineID = SMS_getShineID(SMS_getShineStage(menu->mAreaID), episodeID, false);
    if (shineID == -1) {
        return false;
    }

    return TFlagManager::smInstance->getShineFlag(shineID);
}
SMS_WRITE_32(SMS_PORT_REGION(0x80174B8C, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80174B90, 0, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x80174B94, 0, 0, 0), getShineFlagForSelectScreen2);

static void clampSelectScreenEpisodesVisible() {
    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    menu->mEpisodeCount = Min(menu->mEpisodeCount, sAreaInfos[menu->mAreaID]->mScenarioIDs.size());
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80174E8C, 0, 0, 0), clampSelectScreenEpisodesVisible);
SMS_WRITE_32(SMS_PORT_REGION(0x80174E90, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80174E94, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80174E98, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80174E9C, 0, 0, 0), 0x60000000);

static const char *getScenarioNameForSelectScreen() {
    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    return (const char *)SMSGetMessageData__FPvUl(
        menu->mScenarioBMGData, sAreaInfos[menu->mAreaID]->mScenarioNameIDs[menu->mEpisodeID]);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8017539C, 0, 0, 0), getScenarioNameForSelectScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x8017398c, 0, 0, 0), getScenarioNameForSelectScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x80173a24, 0, 0, 0), getScenarioNameForSelectScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x80173d18, 0, 0, 0), getScenarioNameForSelectScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x80173db0, 0, 0, 0), getScenarioNameForSelectScreen);

const char *loadStageNameFromBMG(void *global_bmg) {
    const char *message;

    const char *errMessage = BetterSMS::isDebugMode() ? "NO DATA" : "";

    s32 area_id = SMS_getShineStage(gpMarDirector->mAreaID);
    message     = (const char *)SMSGetMessageData__FPvUl(global_bmg, area_id);
    return message ? message : errMessage;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80172704, 0x802A0C00, 0, 0), loadStageNameFromBMG);
SMS_PATCH_BL(SMS_PORT_REGION(0x80156D2C, 0x802A0C00, 0, 0), loadStageNameFromBMG);

const char *loadScenarioNameFromBMG(void *global_bmg) {
    const char *message;

    const char *errMessage = BetterSMS::isDebugMode() ? "NO DATA" : "";

    if (sAreaInfos[gpMarDirector->mAreaID] == nullptr) {
        return errMessage;
    }

    s32 episode_id = TFlagManager::smInstance->getFlag(0x40003);
    if (episode_id >= sAreaInfos[gpMarDirector->mAreaID]->mScenarioNameIDs.size()) {
        return errMessage;
    }

    s32 message_idx = sAreaInfos[gpMarDirector->mAreaID]->mScenarioNameIDs[episode_id];

    message = (const char *)SMSGetMessageData__FPvUl(global_bmg, message_idx);
    return message ? message : errMessage;
}
SMS_WRITE_32(SMS_PORT_REGION(0x80172734, 0, 0, 0), 0x4800006C);
SMS_PATCH_BL(SMS_PORT_REGION(0x801727A0, 0x802A0C00, 0, 0), loadScenarioNameFromBMG);

const char *loadScenarioNameFromBMGAfter(void *global_bmg) {
    const char *message;

    const char *errMessage = BetterSMS::isDebugMode() ? "NO DATA" : "";

    if (sAreaInfos[gpMarDirector->mAreaID] == nullptr) {
        return errMessage;
    }

    s32 episode_id = TFlagManager::smInstance->getFlag(0x40003);
    if (episode_id >= sAreaInfos[gpMarDirector->mAreaID]->mScenarioNameIDs.size()) {
        return errMessage;
    }

    s32 message_idx = sAreaInfos[gpMarDirector->mAreaID]->mScenarioNameIDs[episode_id];

    message = (const char *)SMSGetMessageData__FPvUl(global_bmg, message_idx);
    return message ? message : errMessage;
}
SMS_WRITE_32(SMS_PORT_REGION(0x80156D5C, 0, 0, 0), 0x480000A8);
SMS_PATCH_BL(SMS_PORT_REGION(0x80156E04, 0x802A0C00, 0, 0), loadScenarioNameFromBMGAfter);

// Default stage override

static void moveStage_override(TMarDirector *director) {
    director->mNextState           = 5;
    ((u32 *)(director))[0xE4 >> 2] = 15;

    gpApplication.mFader->setColor({0, 0, 0, 255});

    s32 nextStageID = gpApplication.mNextScene.mAreaID;
    s32 thisStageID = gpApplication.mCurrentScene.mAreaID;

    s32 nextShineStage = SMS_getShineStage(nextStageID);
    s32 thisShineStage = SMS_getShineStage(thisStageID);

    if (thisShineStage != nextShineStage) {
        TFlagManager::smInstance->setFlag(0x40002, 0);
    }

    u8 &nextEpisodeID = gpApplication.mNextScene.mEpisodeID;
    s32 episodeID     = TFlagManager::smInstance->getFlag(0x40003);

    if (nextEpisodeID == 0xFF) {
        if (sAreaInfos[nextStageID]) {
            if (nextStageID == 0) {
                nextEpisodeID = 0;
                TFlagManager::smInstance->setFlag(0x40003, 0);
            } else if (nextStageID == 1) {
                ((u32 *)(director))[0xE4 >> 2] = 2;
                nextEpisodeID                  = decideNextScenario__FUc(nextStageID);
                TFlagManager::smInstance->setFlag(0x40003, 0);
            } else if (nextStageID == 7) {
                ((u32 *)(director))[0xE4 >> 2] = 2;
                switch (episodeID) {
                default:
                    nextEpisodeID = 0;
                    break;
                case 1:
                    nextEpisodeID = 0;
                    break;
                case 2:
                    nextEpisodeID = 1;
                    break;
                case 3:
                    nextEpisodeID = 2;
                    break;
                case 4:
                    nextEpisodeID = 2;
                    break;
                case 5:
                    nextEpisodeID = 0;
                    break;
                case 6:
                    nextEpisodeID = 3;
                    break;
                case 7:
                    nextEpisodeID = 4;
                    break;
                }
            } else if (nextStageID == 9) {
                gpApplication.mFader->setColor({210, 210, 210, 255});
                director->mNextState = 8;
            } else {
                ((u32 *)(director))[0xE4 >> 2] = 8;
                director->mNextState           = 8;
            }
        } else {
            if (nextStageID == 13) {
                ((u32 *)(director))[0xE4 >> 2] = 2;
                switch (episodeID) {
                default:
                case 0:
                    nextEpisodeID = 0;
                    break;
                case 2:
                    nextEpisodeID = 1;
                    break;
                case 4:
                    nextEpisodeID = 2;
                    break;
                case 5:
                    nextEpisodeID = 3;
                    break;
                case 6:
                    nextEpisodeID = 3;
                    break;
                case 7:
                    nextEpisodeID = 3;
                    break;
                }
            } else if (nextStageID == 52) {
                ((u32 *)(director))[0xE4 >> 2] = 8;
                nextEpisodeID                  = 0;
                TFlagManager::smInstance->setFlag(0x40003, 0);
            } else if (nextStageID >= 15 && nextStageID < 52) {
                ((u32 *)(director))[0xE4 >> 2] = 2;
                nextEpisodeID                  = 0;
                if (episodeID == 4) {
                    nextEpisodeID = 1;
                } else if (episodeID == 3) {
                    nextEpisodeID = 0;
                }
            } else if (nextStageID == 58) {
                ((u32 *)(director))[0xE4 >> 2] = 2;
                nextEpisodeID                  = 0;
                if (episodeID == 7) {
                    nextEpisodeID = 0;
                } else if (episodeID == 0) {
                    nextEpisodeID = 1;
                }
            } else {
                ((u32 *)(director))[0xE4 >> 2] = 2;
                nextEpisodeID                  = 0;
            }
        }
    }

    if (nextEpisodeID != 0xFF) {
        if ((director->mGameState & 0x100) == 0) {
            director->mNextState = 5;
        } else {
            ((u32 *)(director))[0xE4 >> 2] = 15;
            gpApplication.mFader->setColor({0, 0, 0, 255});
            director->mNextState = 6;
        }
    }

    if (gpMarioAddress->mAttributes.mHasFludd) {
        u8 nozzleType = gpMarioAddress->mFludd->mSecondNozzle;
        if (nozzleType == TWaterGun::Yoshi) {
            nozzleType = TWaterGun::Hover;
        }
        TFlagManager::smInstance->setFlag(0x40004, nozzleType);
    }
}
