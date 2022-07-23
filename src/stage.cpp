#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRNameRef.hxx>
#include <SMS/SMS.hxx>
#include <SMS/game/GameSequence.hxx>
#include <SMS/game/MarDirector.hxx>
#include <SMS/macros.h>
#include <SMS/mapobj/MapObjInit.hxx>

#include "libs/container.hxx"
#include "logging.hxx"
#include "stage.hxx"

#include "common_sdk.h"

using namespace BetterSMS;

static TDictS<Stage::StageInitCallback> sStageInitCBs;
static TDictS<Stage::StageUpdateCallback> sStageUpdateCBs;
static TDictS<Stage::Draw2DCallback> sStageDrawCBs;

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

SMS_NO_INLINE bool BetterSMS::Stage::registerInitCallback(const char *name, StageInitCallback cb) {
    if (sStageInitCBs.hasKey(name))
        return false;
    sStageInitCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Stage::registerUpdateCallback(const char *name,
                                                            StageUpdateCallback cb) {
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

#pragma region MapIdentifiers

static bool isExMap() {
    auto config = Stage::getStageConfiguration();
    if (config->isCustomConfig())
        return config->mIsExStage.get();
    else
        return (gpApplication.mCurrentScene.mAreaID >= TGameSequence::DOLPICEX0 &&
                gpApplication.mCurrentScene.mAreaID <= TGameSequence::COROEX6);
}
SMS_PATCH_B(SMS_PORT_REGION(0x802A8B58, 0x802A0C00, 0, 0), isExMap);

static bool isMultiplayerMap() {
    auto config = Stage::getStageConfiguration();
    if (config->isCustomConfig())
        return config->mIsMultiplayerStage.get();
    else
        return (gpMarDirector->mAreaID == TGameSequence::TEST10 && gpMarDirector->mEpisodeID == 0);
}
SMS_PATCH_B(SMS_PORT_REGION(0x802A8B30, 0x802A0BD8, 0, 0), isMultiplayerMap);

static bool isDivingMap() {
    auto config = Stage::getStageConfiguration();
    if (config->isCustomConfig())
        return config->mIsDivingStage.get();
    else
        return (gpMarDirector->mAreaID == TGameSequence::MAREBOSS ||
                gpMarDirector->mAreaID == TGameSequence::MAREEX0 ||
                gpMarDirector->mAreaID == TGameSequence::MAREUNDERSEA);
}
SMS_PATCH_B(SMS_PORT_REGION(0x802A8AFC, 0x802A0BA4, 0, 0), isDivingMap);

static bool isOptionMap() {
    auto config = Stage::getStageConfiguration();
    if (config->isCustomConfig())
        return config->mIsOptionStage.get();
    else
        return (gpMarDirector->mAreaID == 15);
}
SMS_PATCH_B(SMS_PORT_REGION(0x802A8AE0, 0x802A0B88, 0, 0), isOptionMap);

#pragma endregion

// Extern to stage init
void resetGlobalValues(TMarDirector *) {
    gModelWaterManagerWaterColor.set(0x3C, 0x46, 0x78, 0x14);  // Water rgba
    gYoshiJuiceColor[0].set(0xFE, 0xA8, 0x02, 0x6E);           // Yoshi Juice rgba
    gYoshiJuiceColor[1].set(0x9B, 0x01, 0xFD, 0x6E);
    gYoshiJuiceColor[2].set(0xFD, 0x62, 0xA7, 0x6E);
    gYoshiBodyColor[0].set(0x40, 0xA1, 0x24, 0xFF);  // Yoshi rgba
    gYoshiBodyColor[1].set(0xFF, 0x8C, 0x1C, 0xFF);
    gYoshiBodyColor[2].set(0xAA, 0x4C, 0xFF, 0xFF);
    gYoshiBodyColor[3].set(0xFF, 0xA0, 0xBE, 0xFF);
    gAudioVolume = 0.75f;
    gAudioPitch  = 1.0f;
    gAudioSpeed  = 1.0f;
}

// Extern to stage init
void loadStageConfig(TMarDirector *) {
    Console::debugLog("Reseting stage params...\n");

    Stage::TStageParams *config = Stage::getStageConfiguration();
    config->reset();

    Console::debugLog("Loading stage specific params...\n");
    config->load(Stage::getStageName(&gpApplication));
}