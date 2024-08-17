#include <JGadget/UnorderedMap.hxx>
#include <SMS/GC2D/SelectMenu.hxx>
#include <SMS/Manager/FlagManager.hxx>
#include <SMS/raw_fn.hxx>

#include "module.hxx"
#include "p_area.hxx"

#define MESSAGE_NO_DATA "NO DATA"

static void moveStage_override(TMarDirector *director);

namespace BetterSMS {

    namespace Stage {

        static ShineAreaInfo *sShineAreaInfos[BETTER_SMS_AREA_MAX];
        static NormalAreaInfo sNormalAreaInfos[BETTER_SMS_AREA_MAX];
        static ExAreaInfo sExAreaInfos[BETTER_SMS_EXAREA_MAX];

        static NextStageCallback sNextStageHandler = moveStage_override;

        ShineAreaInfo **getShineAreaInfos() { return sShineAreaInfos; }
        NormalAreaInfo *getNormalAreaInfos() { return sNormalAreaInfos; }
        ExAreaInfo *getExAreaInfos() { return sExAreaInfos; }

    }  // namespace Stage

}  // namespace BetterSMS

using namespace BetterSMS;
using namespace BetterSMS::Stage;

static size_t getScenariosForScene(int sceneID) {
    if (sceneID == 0 || sceneID == 1)
        return 0;

    return 8;
}

static size_t getExScenariosForScene(int sceneID) { return 3; }

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
        JKRHeap *oldHeap = JKRHeap::sRootHeap->becomeCurrentHeap();

        const u32 scenePaneIDs[] = {0,      0, 'bi_0', 'rc_0', 'mm_0', 'pi_0',
                                    'sr_0', 0, 'mo_0', 'mr_0', 0};

        // TODO: Initialize shine stage IDs to -1
        for (size_t i = 0; i < BETTER_SMS_AREA_MAX; ++i) {
            sShineAreaInfos[i]  = nullptr;
            sNormalAreaInfos[i] = {-1};
            sExAreaInfos[i]     = {-1, -1};
        }

        for (int i = 0; i < 64; ++i) {
            ShineAreaInfo *info = sShineAreaInfos[baseGameStageTable[i]];

            bool needs_new_info = info == nullptr;
            if (needs_new_info) {
                info = new ShineAreaInfo(baseGameStageTable[i], scenePaneIDs[i]);
            }

            if (i < 10 && baseGameShineTable[info->getShineStageID()]) {
                for (int j = 0; j < getScenariosForScene(i); ++j) {
                    u8 scenarioID = baseGameShineTable[info->getShineStageID()][j];
                    info->addScenario(scenarioID, baseGameScenarioNameTable[scenarioID]);
                }
            }
            if (i < 10 && baseGameExShineTable[info->getShineStageID()]) {
                // First and last entries always unused
                for (int j = 0; j < getExScenariosForScene(i); ++j) {
                    u8 scenarioID = baseGameExShineTable[info->getShineStageID()][j];
                    info->addExScenario(scenarioID, j > 0 ? baseGameScenarioNameTable[scenarioID] : -1);
                }
            }

            if (needs_new_info) {
                registerShineStage(info);
            }

            registerNormalStage(i, info->getShineStageID());
        }

        for (int i = 0; i < 32; ++i) {
            registerExStage(i + 0x15, baseGameStageTable[i + 0x15],
                            baseGameExShineTable2[i] != 0xFF ? baseGameExShineTable2[i] : -1);
        }

        oldHeap->becomeCurrentHeap();
    }
}

bool BetterSMS::Stage::registerShineStage(ShineAreaInfo *info) {
    if (sShineAreaInfos[info->getShineStageID()]) {
        OSReport("[WARN] Overwriting stage info for stage %d\n", info->getShineStageID());
    }
    sShineAreaInfos[info->getShineStageID()] = info;
    return true;
}

bool BetterSMS::Stage::registerNormalStage(u8 normalStageID, u8 shineStageID) {
    if (sNormalAreaInfos[normalStageID].mShineStageID != -1) {
        OSReport("[WARN] Overwriting stage info for stage %d\n", normalStageID);
    }
    sNormalAreaInfos[normalStageID] = {shineStageID};
    return true;
}

bool BetterSMS::Stage::registerExStage(u8 exStageID, u8 shineStageID, s32 shineID) {
    if (sExAreaInfos[exStageID].mShineStageID != -1) {
        OSReport("[WARN] Overwriting ex stage info for stage %d\n", exStageID);
    }
    sExAreaInfos[exStageID] = {shineStageID, shineID};
    return registerNormalStage(exStageID, shineStageID);
}

void BetterSMS::Stage::setNextStageHandler(NextStageCallback callback) {
    sNextStageHandler = callback;
}

static void moveStageHandler(TMarDirector *director) { sNextStageHandler(director); }
SMS_PATCH_BL(SMS_PORT_REGION(0x80297E40, 0, 0, 0), moveStageHandler);
SMS_PATCH_BL(SMS_PORT_REGION(0x80299244, 0, 0, 0), moveStageHandler);
SMS_PATCH_BL(SMS_PORT_REGION(0x8029933C, 0, 0, 0), moveStageHandler);
SMS_PATCH_BL(SMS_PORT_REGION(0x8029946C, 0, 0, 0), moveStageHandler);

static s32 SMS_getShineID(u32 stageID, u32 scenarioID, bool isExStage) {
    if (!sShineAreaInfos[stageID]) {
        return -1;
    }
    const ShineAreaInfo &info = *sShineAreaInfos[stageID];
    if (isExStage) {
        const TGlobalVector<s32> &exScenarioIDs = info.getExScenarioIDs();
        if (scenarioID >= exScenarioIDs.size()) {
            return -1;
        }
        return exScenarioIDs[scenarioID];
    } else {
        const TGlobalVector<s32> &scenarioIDs = info.getScenarioIDs();
        if (scenarioID >= scenarioIDs.size()) {
            return -1;
        }
        return scenarioIDs[scenarioID];
    }
}
SMS_PATCH_B(SMS_PORT_REGION(0x8016FAC0, 0, 0, 0), SMS_getShineID);
SMS_PATCH_B(SMS_PORT_REGION(0x80175AF8, 0, 0, 0), SMS_getShineID);
SMS_PATCH_B(SMS_PORT_REGION(0x8017CC6C, 0, 0, 0), SMS_getShineID);

static s32 SMS_getShineIDofExStage(u32 exStageID) {
    if (sExAreaInfos[exStageID].mShineStageID == -1)
        return -1;
    return sExAreaInfos[exStageID].mShineID;
}
SMS_PATCH_B(SMS_PORT_REGION(0x802a8a98, 0, 0, 0), SMS_getShineIDofExStage);

static s32 SMS_getShineStage(u32 stageID) { return sNormalAreaInfos[stageID].mShineStageID; }
SMS_PATCH_B(SMS_PORT_REGION(0x802A8AC8, 0, 0, 0), SMS_getShineStage);

static TExPane *constructExPaneForSelectScreen(TExPane *pane, J2DScreen *screen) {
    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    // This check is only necessary once
    SMS_ASSERT(sShineAreaInfos[SMS_getShineStage(menu->mAreaID)]->getShineSelectPaneID() != 0,
               "Tried to open shine select screen for an area that has no pane ID!");

    return (TExPane *)__ct__7TExPaneFP9J2DScreenUl(
        pane, screen, sShineAreaInfos[SMS_getShineStage(menu->mAreaID)]->getShineSelectPaneID());
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80174D40, 0, 0, 0), constructExPaneForSelectScreen);

static TBoundPane *constructBoundPaneForSelectScreenA(TBoundPane *pane, J2DScreen *screen) {
    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    u32 paneID =
        (sShineAreaInfos[SMS_getShineStage(menu->mAreaID)]->getShineSelectPaneID() & 0xFFFFFF00) |
        'a';

    return (TBoundPane *)__ct__10TBoundPaneFP9J2DScreenUl(pane, screen, paneID);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80174D88, 0, 0, 0), constructBoundPaneForSelectScreenA);

static TBoundPane *constructBoundPaneForSelectScreenB(TBoundPane *pane, J2DScreen *screen) {
    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    u32 paneID =
        (sShineAreaInfos[SMS_getShineStage(menu->mAreaID)]->getShineSelectPaneID() & 0xFFFFFF00) |
        'b';

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
    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    int index;
    SMS_FROM_GPR(26, index);

    int shineID = SMS_getShineID(SMS_getShineStage(menu->mAreaID), index, true);
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

    ShineAreaInfo *info = sShineAreaInfos[SMS_getShineStage(menu->mAreaID)];
    if (!info) {
        return;
    }

    const TGlobalVector<s32> &scenarioIDs = info->getScenarioIDs();
    menu->mEpisodeCount                   = Min(menu->mEpisodeCount, scenarioIDs.size());
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80174E8C, 0, 0, 0), clampSelectScreenEpisodesVisible);
SMS_WRITE_32(SMS_PORT_REGION(0x80174E90, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80174E94, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80174E98, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80174E9C, 0, 0, 0), 0x60000000);

static const char *getScenarioNameForSelectScreen() {
    TSelectMenu *menu;
    SMS_FROM_GPR(31, menu);

    ShineAreaInfo *info = sShineAreaInfos[SMS_getShineStage(menu->mAreaID)];
    if (!info) {
        return MESSAGE_NO_DATA;
    }

    const TGlobalVector<s32> &scenarioNameIDs = info->getScenarioNameIDs();
    if (menu->mEpisodeID >= scenarioNameIDs.size()) {
        return MESSAGE_NO_DATA;
    }

    s32 message_idx = scenarioNameIDs[menu->mEpisodeID];
    if (message_idx == -1) {
        return MESSAGE_NO_DATA;
    }

    return (const char *)SMSGetMessageData__FPvUl(menu->mScenarioBMGData, message_idx);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8017539C, 0, 0, 0), getScenarioNameForSelectScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x8017398C, 0, 0, 0), getScenarioNameForSelectScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x80173A24, 0, 0, 0), getScenarioNameForSelectScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x80173D18, 0, 0, 0), getScenarioNameForSelectScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x80173DB0, 0, 0, 0), getScenarioNameForSelectScreen);

const char *loadStageNameFromBMG(void *global_bmg) {
    const char *message;

    const char *errMessage = BetterSMS::isDebugMode() ? MESSAGE_NO_DATA : nullptr;

    if (gpMarDirector->mAreaID >= 60) {
        TFlagManager::smInstance->setFlag(0x40003, gpMarDirector->mEpisodeID);
    }

    s32 area_id = SMS_getShineStage(gpMarDirector->mAreaID);
    message     = (const char *)SMSGetMessageData__FPvUl(global_bmg, area_id);
    return message ? message : errMessage;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80172704, 0x802A0C00, 0, 0), loadStageNameFromBMG);
SMS_PATCH_BL(SMS_PORT_REGION(0x80156D2C, 0x802A0C00, 0, 0), loadStageNameFromBMG);

static const char *loadScenarioNameFromBMG(void *global_bmg) {
    const char *message;

    const char *errMessage = BetterSMS::isDebugMode() ? MESSAGE_NO_DATA : nullptr;

    ShineAreaInfo *info = sShineAreaInfos[SMS_getShineStage(gpMarDirector->mAreaID)];
    if (!info) {
        return errMessage;
    }

    const TGlobalVector<s32> &scenarioNameIDs = info->getScenarioNameIDs();

    s32 episode_id = TFlagManager::smInstance->getFlag(0x40003);
    if (episode_id >= scenarioNameIDs.size()) {
        return errMessage;
    }

    s32 message_idx = scenarioNameIDs[episode_id];
    if (message_idx == -1) {
        return MESSAGE_NO_DATA;
    }

    message = (const char *)SMSGetMessageData__FPvUl(global_bmg, message_idx);
    return message ? message : errMessage;
}
SMS_WRITE_32(SMS_PORT_REGION(0x80172734, 0, 0, 0), 0x4800006C);
SMS_PATCH_BL(SMS_PORT_REGION(0x801727A0, 0x802A0C00, 0, 0), loadScenarioNameFromBMG);

static const char *loadScenarioNameFromBMGAfter(void *global_bmg) {
    const char *message;

    const char *errMessage = BetterSMS::isDebugMode() ? MESSAGE_NO_DATA : nullptr;

    ShineAreaInfo *info = sShineAreaInfos[SMS_getShineStage(gpMarDirector->mAreaID)];
    if (!info) {
        return errMessage;
    }

    const TGlobalVector<s32> &scenarioNameIDs = info->getScenarioNameIDs();

    s32 episode_id = TFlagManager::smInstance->getFlag(0x40003);
    if (episode_id >= scenarioNameIDs.size()) {
        return errMessage;
    }

    s32 message_idx = scenarioNameIDs[episode_id];
    if (message_idx == -1) {
        return MESSAGE_NO_DATA;
    }

    message = (const char *)SMSGetMessageData__FPvUl(global_bmg, message_idx);
    return message ? message : errMessage;
}

SMS_NO_INLINE static const char *loadScenarioNameFromBMGAfterStub(u8 *pause_menu, void* global_bmg) {
    const char *name = loadScenarioNameFromBMGAfter(global_bmg);
    if (!name) {
        (*(J2DPane **)(pause_menu + 0x1C))->add(0, 30);
        (*(J2DPane **)(pause_menu + 0xD4))->add(0, 15);
    }

    return name;
}

// Stupid register bullshit
static const char *loadScenarioNameFromBMGAfterStubStub(void *global_bmg) {
    u8 *pause_menu;
    SMS_FROM_GPR(29, pause_menu);

    return loadScenarioNameFromBMGAfterStub(pause_menu, global_bmg);
}
SMS_WRITE_32(SMS_PORT_REGION(0x80156D5C, 0, 0, 0), 0x480000A8);
SMS_PATCH_BL(SMS_PORT_REGION(0x80156E04, 0x802A0C00, 0, 0), loadScenarioNameFromBMGAfterStubStub);

// Default stage override

static void moveStage_override(TMarDirector *director) {
    if (gpApplication.mNextScene.mAreaID <= 60 || gpApplication.mNextScene.mEpisodeID != 0xFF) {
        director->moveStage();
        return;
    }

    if (sNormalAreaInfos[gpApplication.mNextScene.mAreaID].mShineStageID == -1) {
        director->moveStage();
        return;
    }

    s32 shineStageID = SMS_getShineStage(gpApplication.mNextScene.mAreaID);

    bool isSameShineStage = SMS_getShineStage(gpApplication.mCurrentScene.mAreaID) == shineStageID;
    bool isSameNormalStage =
        gpApplication.mCurrentScene.mAreaID == gpApplication.mNextScene.mAreaID;

    if (isSameShineStage && !isSameNormalStage && !TFlagManager::smInstance->getBool(0x50010)) {
        if (gpApplication.mNextScene.mEpisodeID == 0xFF) {
            gpApplication.mNextScene.mEpisodeID = gpApplication.mCurrentScene.mEpisodeID;
            director->moveStage();
            return;
        }
    }

    gpApplication.mFader->setColor({0, 0, 0, 255});

    if (SMS_getShineStage(gpApplication.mNextScene.mAreaID) !=
        SMS_getShineStage(gpApplication.mCurrentScene.mAreaID)) {
        TFlagManager::smInstance->setFlag(0x40002, 0);
    }

    *(u32 *)((u8 *)director + 0xE4) = 8;
    director->mNextState            = 8;
}
