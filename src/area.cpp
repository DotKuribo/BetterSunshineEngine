#include "area.hxx"
#include "module.hxx"
#include <JGadget/UnorderedMap.hxx>
#include <SMS/GC2D/SelectMenu.hxx>
#include <SMS/Manager/FlagManager.hxx>
#include <SMS/raw_fn.hxx>

// TODO: Initialize AreaInfo structs for each stage in base game.
// Custom registration should overwrite these defaults.

static BetterSMS::Stage::AreaInfo *sAreaInfos[256];
static BetterSMS::Stage::ExAreaInfo *sExAreaInfos[256];

static size_t getScenariosForScene(int sceneID) {
    if (sceneID == 0)
        return 4;

    return 8;
}

static size_t getExScenariosForScene(int sceneID) { return 4; }

BETTER_SMS_FOR_CALLBACK void initAreaInfo(TApplication *) {
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

        const u32 scenePaneIDs[] = {0xFFFFFFFF, 0xFFFFFFFF, 'bi_0', 'rc_0', 'mm_0',
                                    'yi_0',     'sr_0',     'mo_0', 'mr_0', 0xFFFFFFFF};

        for (int i = 0; i < 10; ++i) {
            auto info                = new BetterSMS::Stage::AreaInfo;
            info->mShineSelectPaneID = scenePaneIDs[i];
            if (baseGameShineTable[i]) {
                info->mShineStageID  = baseGameStageTable[i];
                info->mNormalStageID = baseGameNormalStageTable[i];
                for (int j = 0; j < getScenariosForScene(i); ++j) {
                    auto scenarioID = baseGameShineTable[i][j];
                    info->mScenarioIDs.push_back(baseGameShineTable[i][j]);
                    info->mScenarioNameIDs.push_back(baseGameScenarioNameTable[scenarioID]);
                }
            }
            if (baseGameExShineTable[i]) {
                for (int j = 0; j < getExScenariosForScene(i); ++j) {
                    auto scenarioID = baseGameExShineTable[i][j];
                    info->mExScenarioIDs.push_back(baseGameExShineTable[i][j]);
                    info->mExScenarioNameIDs.push_back(baseGameScenarioNameTable[scenarioID]);
                }
            }
            BetterSMS::Stage::registerStageInfo(i, info);
        }

        for (int i = 0; i < 32; ++i) {
            auto exInfo = new BetterSMS::Stage::ExAreaInfo;
            exInfo->mParentStageID = -1;
            exInfo->mShineID       = baseGameExShineTable2[i];
            BetterSMS::Stage::registerExStageInfo(i + 21, exInfo);
        }

        oldHeap->becomeCurrentHeap();
    }
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Stage::registerStageInfo(u8 stageID, AreaInfo *info) {
    if (sAreaInfos[stageID]) {
        return false;
    }
    sAreaInfos[stageID] = info;
    return true;
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Stage::deregisterStageInfo(u8 stageID) {
    sAreaInfos[stageID] = nullptr;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Stage::registerExStageInfo(u8 stageID, ExAreaInfo *info) {
    if (sExAreaInfos[stageID]) {
        return false;
    }
    sExAreaInfos[stageID] = info;
    return true;
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Stage::deregisterExStageInfo(u8 stageID) {
    sExAreaInfos[stageID] = nullptr;
}

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

static u8 SMS_getShineIDofExStage(u32 exStageID) {
    if (!sExAreaInfos[exStageID])
        return 0xFF;
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

static bool getShineFlagForSelectScreen() {
    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    int shineID = SMS_getShineID(SMS_getShineStage(menu->mAreaID), menu->mEpisodeID, false);
    if (shineID == -1) {
        return false;
    }

    return TFlagManager::smInstance->getShineFlag(shineID);
}
SMS_WRITE_32(SMS_PORT_REGION(0x80174B40, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80174B44, 0, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x80174B48, 0, 0, 0), getShineFlagForSelectScreen);
SMS_WRITE_32(SMS_PORT_REGION(0x80174B8C, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80174B90, 0, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x80174B94, 0, 0, 0), getShineFlagForSelectScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x80174E58, 0, 0, 0), getShineFlagForSelectScreen);

static const char *getScenarioNameForSelectScreen() {
    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    int stageID = SMS_getShineStage(menu->mAreaID);
    if (stageID == -1) {
        return nullptr;
    }

    return (const char *)SMSGetMessageData__FPvUl(
        menu->mScenarioBMGData, sAreaInfos[stageID]->mScenarioNameIDs[menu->mEpisodeID]);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8017539C, 0, 0, 0), getScenarioNameForSelectScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x8017398c, 0, 0, 0), getScenarioNameForSelectScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x80173a24, 0, 0, 0), getScenarioNameForSelectScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x80173d18, 0, 0, 0), getScenarioNameForSelectScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x80173db0, 0, 0, 0), getScenarioNameForSelectScreen);

static void moveStage_override(TMarDirector *director) {
    director->mNextState           = 5;
    ((u32 *)(director))[0xE4 >> 2] = 15;

    gpApplication.mFader->setColor({0, 0, 0, 255});

    s32 nextStageID = SMS_getShineStage(gpApplication.mNextScene.mAreaID);
    s32 curStageID  = SMS_getShineStage(gpApplication.mCurrentScene.mAreaID);
    if (curStageID != nextStageID) {
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
SMS_PATCH_B(SMS_PORT_REGION(0x80297584, 0, 0, 0), moveStage_override);