#include <Dolphin/DVD.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRNameRef.hxx>

#include <SMS/System/Application.hxx>
#include <SMS/System/GameSequence.hxx>
#include <SMS/System/MarDirector.hxx>
#include <SMS/macros.h>
#include <SMS/Manager/ModelWaterManager.hxx>
#include <SMS/MapObj/MapObjInit.hxx>

#include "libs/container.hxx"
#include "loading.hxx"
#include "logging.hxx"
#include "stage.hxx"
#include "module.hxx"

using namespace BetterSMS;

static TDictS<Stage::InitCallback> sStageInitCBs;
static TDictS<Stage::UpdateCallback> sStageUpdateCBs;
static TDictS<Stage::Draw2DCallback> sStageDrawCBs;
static TDictS<Stage::ExitCallback> sStageExitCBs;

Stage::TStageParams *Stage::TStageParams::sStageConfig = nullptr;

SMS_NO_INLINE Stage::TStageParams *BetterSMS::Stage::getStageConfiguration() {
    return TStageParams::sStageConfig;
}

SMS_NO_INLINE const char *BetterSMS::Stage::getStageName(TApplication *application) {
    AreaEpisodeArray *AreaPathArray = application->mStringPaths;

    if (!AreaPathArray)
        return nullptr;

    u32 *AreaArrayStart = AreaPathArray->startArray;

    if (!AreaArrayStart || (((u32)AreaPathArray->endArray - (u32)AreaArrayStart) >> 2) <
                               (u8)application->mCurrentScene.mAreaID)
        return nullptr;

    AreaEpisodeArray *StagePathArray =
        (AreaEpisodeArray *)AreaArrayStart[(u8)application->mCurrentScene.mAreaID];
    u32 *StageArrayStart = (u32 *)StagePathArray->startArray;

    if (!StageArrayStart || (((u32)StagePathArray->endArray - (u32)StageArrayStart) >> 4) <
                                application->mCurrentScene.mEpisodeID)
        return nullptr;

    return (char *)(StageArrayStart[(application->mCurrentScene.mEpisodeID << 2) + (0xC / 4)]);
}

SMS_NO_INLINE bool BetterSMS::Stage::isStageInitRegistered(const char *name) {
    return sStageInitCBs.hasKey(name);
}
SMS_NO_INLINE bool BetterSMS::Stage::isStageUpdateRegistered(const char *name) {
    return sStageUpdateCBs.hasKey(name);
}
SMS_NO_INLINE bool BetterSMS::Stage::isDraw2DRegistered(const char *name) {
    return sStageDrawCBs.hasKey(name);
}

SMS_NO_INLINE bool BetterSMS::Stage::registerInitCallback(const char *name, InitCallback cb) {
    if (sStageInitCBs.hasKey(name))
        return false;
    sStageInitCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Stage::registerUpdateCallback(const char *name, UpdateCallback cb) {
    if (sStageUpdateCBs.hasKey(name))
        return false;
    sStageUpdateCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Stage::registerDraw2DCallback(const char *name, Draw2DCallback cb) {
    if (sStageDrawCBs.hasKey(name))
        return false;
    sStageDrawCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Stage::registerExitCallback(const char *name, ExitCallback cb) {
    if (sStageExitCBs.hasKey(name))
        return false;
    sStageExitCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Stage::deregisterInitCallback(const char *name) {
    if (!sStageDrawCBs.hasKey(name))
        return false;
    sStageDrawCBs.pop(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Stage::deregisterUpdateCallback(const char *name) {
    if (!sStageDrawCBs.hasKey(name))
        return false;
    sStageDrawCBs.pop(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Stage::deregisterDraw2DCallback(const char *name) {
    if (!sStageDrawCBs.hasKey(name))
        return false;
    sStageDrawCBs.pop(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Stage::deregisterExitCallback(const char *name) {
    if (!sStageExitCBs.hasKey(name))
        return false;
    sStageExitCBs.pop(name);
    return true;
}

void BetterSMS::Stage::TStageParams::reset() {
    mIsExStage.set(false);
    mIsDivingStage.set(false);
    mIsOptionStage.set(false);
    mIsMultiplayerStage.set(false);
    mIsEggFree.set(true);
    // mLightType.set(TLightContext::ActiveType::DISABLED);
    // mLightPosX.set(0.0f);
    // mLightPosY.set(3600.0f);
    // mLightPosZ.set(-7458.0f);
    // mLightSize.set(8000.0f);
    // mLightStep.set(100.0f);
    // mLightColor.set(JUtility::TColor(0, 20, 40, 0));
    // mLightLayerCount.set(5);
    // mLightDarkLevel.set(120);
    // mPlayerSelectWhiteList.set(0xFFFFFFFF);
    mPlayerHasFludd.set(true);
    mPlayerHasHelmet.set(false);
    mPlayerHasGlasses.set(false);
    mPlayerHasShirt.set(false);
    mPlayerCanRideYoshi.set(true);
}

static int findNumber(const char *string, size_t max) {
    for (int i = 0; i < max; ++i) {
        const char chr = string[i];
        if (chr == '\0')
            return -1;
        if (chr >= 0x30 && chr <= 0x39)
            return i;
    }
    return -1;
}

static int findExtension(const char *string, size_t max) {
    for (int i = 0; i < max; ++i) {
        const char chr = string[i];
        if (chr == '\0')
            return -1;
        if (chr == '.')
            return i;
    }
    return -1;
}

char *BetterSMS::Stage::TStageParams::stageNameToParamPath(char *dst, const char *stage,
                                                           bool generalize) {
    strncpy(dst, "/data/scene/System/", 20);

    const int numIDPos = findNumber(stage, 60);
    if (generalize && numIDPos != -1) {
        strncpy(dst + 19, stage, numIDPos);
        dst[19 + numIDPos] = '\0';
        strcat(dst, "+.prm");
    } else {
        const int extensionPos = findExtension(stage, 60);
        if (extensionPos == -1)
            strcat(dst, stage);
        else
            strncpy(dst + 19, stage, extensionPos);
        dst[19 + extensionPos] = '\0';
        strcat(dst, ".prm");
    }

    return dst;
}

void BetterSMS::Stage::TStageParams::load(const char *stageName) {
    DVDFileInfo fileInfo;
    s32 entrynum;

    char path[64];
    stageNameToParamPath(path, stageName, false);

    entrynum = DVDConvertPathToEntrynum(path);
    if (entrynum >= 0) {
        DVDFastOpen(entrynum, &fileInfo);
        void *buffer = JKRHeap::alloc(fileInfo.mLen, 32, nullptr);

        DVDReadPrio(&fileInfo, buffer, fileInfo.mLen, 0, 2);
        DVDClose(&fileInfo);
        {
            JSUMemoryInputStream stream(buffer, fileInfo.mLen);
            TParams::load(stream);
            JKRHeap::free(buffer, nullptr);
        }
        mIsCustomConfigLoaded = true;
        return;
    }

    stageNameToParamPath(path, stageName, true);

    entrynum = DVDConvertPathToEntrynum(path);
    if (entrynum >= 0) {
        DVDFastOpen(entrynum, &fileInfo);
        void *buffer = JKRHeap::alloc(fileInfo.mLen, 32, nullptr);

        DVDReadPrio(&fileInfo, buffer, fileInfo.mLen, 0, 2);
        DVDClose(&fileInfo);
        {
            JSUMemoryInputStream stream(buffer, fileInfo.mLen);
            TParams::load(stream);
            JKRHeap::free(buffer, nullptr);
        }
        mIsCustomConfigLoaded = true;
        return;
    }

    mIsCustomConfigLoaded = false;
}

extern void updateDebugCallbacks();

void initStageLoading(TMarDirector *director) {
    Loading::setLoading(true);
    director->loadResource();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80296DE0, 0x80291750, 0, 0), initStageLoading);

void initStageCallbacks(TMarDirector *director) {
    TDictS<Stage::InitCallback>::ItemList stageInitCBs;
    sStageInitCBs.items(stageInitCBs);

    for (auto &item : stageInitCBs) {
        item.mValue(director);
    }

    director->setupObjects();
    Loading::setLoading(false);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802998B8, 0x80291750, 0, 0), initStageCallbacks);

void updateStageCallbacks(TApplication *app) {
    u32 func;
    SMS_FROM_GPR(12, func);

    if (gpMarDirector && app->mContext == TApplication::CONTEXT_DIRECT_STAGE) {
        TDictS<Stage::UpdateCallback>::ItemList stageUpdateCBs;
        sStageUpdateCBs.items(stageUpdateCBs);

        for (auto &item : stageUpdateCBs) {
            item.mValue(gpMarDirector);
        }
    }
}

void drawStageCallbacks(J2DOrthoGraph *ortho) {
    ortho->setup2D();

    TDictS<Stage::Draw2DCallback>::ItemList stageDrawCBs;
    sStageDrawCBs.items(stageDrawCBs);

    for (auto &item : stageDrawCBs) {
        item.mValue(gpMarDirector, ortho);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80143F14, 0x80138B50, 0, 0), drawStageCallbacks);

void exitStageCallbacks(TApplication *app) {
    if (app->mContext != TApplication::CONTEXT_DIRECT_STAGE)
        return;

    TDictS<Stage::ExitCallback>::ItemList stageExitCBs;
    sStageExitCBs.items(stageExitCBs);

    for (auto &item : stageExitCBs) {
        item.mValue(app);
    }

    delete Stage::getStageConfiguration();
}

#pragma region MapIdentifiers

static bool isExMap() {
    auto config = Stage::getStageConfiguration();
    if (config && config->isCustomConfig())
        return config->mIsExStage.get();
    else
        return (gpApplication.mCurrentScene.mAreaID >= TGameSequence::DOLPICEX0 &&
                gpApplication.mCurrentScene.mAreaID <= TGameSequence::COROEX6);
}
SMS_PATCH_B(SMS_PORT_REGION(0x802A8B58, 0x802A0C00, 0, 0), isExMap);

static bool isMultiplayerMap() {
    auto config = Stage::getStageConfiguration();
    if (config && config->isCustomConfig())
        return config->mIsMultiplayerStage.get();
    else
        return (gpMarDirector->mAreaID == TGameSequence::TEST10 && gpMarDirector->mEpisodeID == 0);
}
SMS_PATCH_B(SMS_PORT_REGION(0x802A8B30, 0x802A0BD8, 0, 0), isMultiplayerMap);

static bool isDivingMap() {
    auto config = Stage::getStageConfiguration();
    if (config && config->isCustomConfig())
        return config->mIsDivingStage.get();
    else
        return (gpMarDirector->mAreaID == TGameSequence::MAREBOSS ||
                gpMarDirector->mAreaID == TGameSequence::MAREEX0 ||
                gpMarDirector->mAreaID == TGameSequence::MAREUNDERSEA);
}
SMS_PATCH_B(SMS_PORT_REGION(0x802A8AFC, 0x802A0BA4, 0, 0), isDivingMap);

static bool isOptionMap() {
    auto config = Stage::getStageConfiguration();
    if (config && config->isCustomConfig())
        return config->mIsOptionStage.get();
    else
        return (gpMarDirector->mAreaID == 15);
}
SMS_PATCH_B(SMS_PORT_REGION(0x802A8AE0, 0x802A0B88, 0, 0), isOptionMap);

#pragma endregion

// Extern to stage init
void loadStageConfig(TMarDirector *) {
    Console::debugLog("Reseting stage params...\n");

    Stage::TStageParams::sStageConfig = new (JKRHeap::sSystemHeap, 4) Stage::TStageParams(nullptr);

    Stage::TStageParams *config = Stage::getStageConfiguration();
    config->reset();

    Console::debugLog("Loading stage specific params...\n");
    config->load(Stage::getStageName(&gpApplication));
}

// Extern to stage init
void resetGlobalValues(TApplication *) {
    waterColor[0].set(0x3C, 0x46, 0x78, 0x14);  // Water rgba
    waterColor[1].set(0xFE, 0xA8, 0x02, 0x6E);  // Yoshi Juice rgba
    waterColor[2].set(0x9B, 0x01, 0xFD, 0x6E);
    waterColor[3].set(0xFD, 0x62, 0xA7, 0x6E);
    bodyColor[0].set(0x40, 0xA1, 0x24, 0xFF);  // Yoshi rgba
    bodyColor[1].set(0xFF, 0x8C, 0x1C, 0xFF);
    bodyColor[2].set(0xAA, 0x4C, 0xFF, 0xFF);
    bodyColor[3].set(0xFF, 0xA0, 0xBE, 0xFF);
    gAudioVolume = 0.75f;
    gAudioPitch  = 1.0f;
    gAudioSpeed  = 1.0f;
}