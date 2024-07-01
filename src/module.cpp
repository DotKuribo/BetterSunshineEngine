#include <Dolphin/OS.h>
#include <Dolphin/math.h>

#include <SMS/macros.h>

#include "libs/global_unordered_map.hxx"
#include "libs/string.hxx"

// BetterSMS API
#include "application.hxx"
#include "area.hxx"
#include "bmg.hxx"
#include "cstd/ctype.h"
#include "cstd/stdlib.h"
#include "debug.hxx"
#include "game.hxx"
#include "libs/optional.hxx"
#include "loading.hxx"
#include "logging.hxx"
#include "memory.hxx"
#include "module.hxx"
#include "music.hxx"
#include "object.hxx"
#include "objects/fog.hxx"
#include "objects/generic.hxx"
#include "objects/particle.hxx"
#include "objects/sound.hxx"
#include "p_module.hxx"
#include "p_settings.hxx"
#include "player.hxx"
#include "settings.hxx"
#include "stage.hxx"
#include "time.hxx"

// SETTINGS //

// BetterSMS settings
static Settings::SettingsGroup sSettingsGroup(2, 0, Settings::Priority::CORE);

BugsSetting gBugFixesSetting("Bug Fixes");
BugsSetting gExploitFixesSetting("Exploit Fixes");
CollisionFixesSetting gCollisionFixesSetting("Collision Fixes");
AspectRatioSetting gAspectRatioSetting("Aspect Ratio");
ViewportSetting gViewportSetting("Viewport");
FPSSetting gFPSSetting("Frame Rate");
static bool sCameraInvertX = false;
static bool sCameraInvertY = false;
Settings::SwitchSetting gCameraInvertXSetting("Invert Camera X", &sCameraInvertX);
Settings::SwitchSetting gCameraInvertYSetting("Invert Camera Y", &sCameraInvertY);
SavePromptsSetting gSavePromptSetting("Save Prompts");

FirstBootSetting gFirstBootSetting;

static BetterSMS::ModuleInfo sBetterSMSInfo{"Better Sunshine Engine", 1, 2, &sSettingsGroup};
//

BetterSMS::TGlobalVector<BetterSMS::ModuleInfo> gModuleInfos;

optional<ModuleInfo> BetterSMS::getModuleInfo(const char *key) {
    for (auto &info : gModuleInfos) {
        if (strcmp(info.mName, key) == 0) {
            return info;
        }
    }
    return {};
}

bool BetterSMS::isModuleRegistered(const char *key) {
    for (auto &info : gModuleInfos) {
        if (strcmp(info.mName, key) == 0) {
            return true;
        }
    }
    return false;
}

bool BetterSMS::registerModule(const ModuleInfo &info) {
    if (isModuleRegistered(info.mName)) {
        OSPanic(__FILE__, __LINE__,
                "Module \"%s\" is trying to register under the name \"%s\", which is "
                "already taken!",
                info.mName, info.mName);
        return false;
    }
    gModuleInfos.push_back(info);
    return true;
}

// ================================= //

using namespace BetterSMS;

// OBJECT
extern void makeExtendedObjDataTable();

extern bool BetterAppContextGameBoot(TApplication *app);
extern bool BetterAppContextGameBootLogo(TApplication *app);
extern bool BetterAppContextDirectMovie(TApplication *app);
extern bool BetterAppContextGameBootIntro(TApplication *app);
extern bool BetterAppContextDirectStage(TApplication *app);
extern bool BetterAppContextDirectShineSelect(TApplication *app);
extern bool BetterAppContextDirectLevelSelect(TApplication *app);
extern bool BetterAppContextDirectSettingsMenu(TApplication *app);

// DEBUG
extern void initDebugCallbacks(TApplication *);
extern void resetDebugState(TMarDirector *);

extern u32 XYZState;
extern void checkDebugMode(TMario *, bool);
extern bool updateDebugMode(TMario *);
extern void updateFluddNozzle(TApplication *);

extern void drawMonitor(TApplication *, const J2DOrthoGraph *);
extern void resetMonitor(TApplication *);

extern void initFPSMonitor(TApplication *);
extern void updateFPSMonitor(TApplication *);
extern void drawFPSMonitor(TApplication *, const J2DOrthoGraph *);

extern void initDebugStateMonitor(TApplication *);
extern void updateDebugStateMonitor(TApplication *);
extern void drawDebugStateMonitor(TApplication *, const J2DOrthoGraph *);

extern void initGameStateMonitor(TApplication *);
extern void updateGameStateMonitor(TApplication *);
extern void drawGameStateMonitor(TApplication *, const J2DOrthoGraph *);

// GAME
extern void extendLightEffectToShineCount(TApplication *app);

// PLAYER MOVES
extern u32 MultiJumpState;
extern void updateDeadTriggerState(TMario *player, bool isMario);

// PLAYER STATE
extern void updateCollisionContext(TMario *, bool);
extern void updateClimbContext(TMario *, bool);
extern void checkForForceDropOnDeadActor(TMario *, bool);

// PLAYER ANIM
extern void initExtendedPlayerAnims();

// PLAYER COLLISION
extern void initializeWarpCallback(TMario *, bool);
extern void processWarpCallback(TMario *, bool);

extern void decHealth(TMario *player, const TBGCheckData *data, u32 flags);
extern void incHealth(TMario *player, const TBGCheckData *data, u32 flags);
extern void elecPlayer(TMario *player, const TBGCheckData *data, u32 flags);
extern void burnPlayer(TMario *player, const TBGCheckData *data, u32 flags);
extern void changeNozzleSprayOnTouch(TMario *player, const TBGCheckData *data, u32 flags);
extern void changeNozzleHoverOnTouch(TMario *player, const TBGCheckData *data, u32 flags);
extern void changeNozzleTurboOnTouch(TMario *player, const TBGCheckData *data, u32 flags);
extern void changeNozzleRocketOnTouch(TMario *player, const TBGCheckData *data, u32 flags);
extern void changeNozzleSpray(TMario *player, const TBGCheckData *data, u32 flags);
extern void changeNozzleHover(TMario *player, const TBGCheckData *data, u32 flags);
extern void changeNozzleTurbo(TMario *player, const TBGCheckData *data, u32 flags);
extern void changeNozzleRocket(TMario *player, const TBGCheckData *data, u32 flags);
extern void setGravityCol(TMario *player, const TBGCheckData *data, u32 flags);
extern void antiGravityCol(TMario *player, const TBGCheckData *data, u32 flags);
extern void boostPadCol(TMario *player, const TBGCheckData *data, u32 flags);
extern void instantWarpHandler(TMario *player, const TBGCheckData *data, u32 flags);
extern void screenWipeWarpHandler(TMario *player, const TBGCheckData *data, u32 flags);
extern void instantScreenWipeWarpHandler(TMario *player, const TBGCheckData *data, u32 flags);
extern void effectWarpHandler(TMario *player, const TBGCheckData *data, u32 flags);
extern void portalWarpHandler(TMario *player, const TBGCheckData *data, u32 flags);
extern void portalFreeWarpHandler(TMario *player, const TBGCheckData *data, u32 flags);

// YOSHI
extern void checkForYoshiDeath(TMario *player, bool isMario);
extern void forceValidRidingAnimation(TMario *player, bool isMario);

// MUSIC
extern void stopMusicOnExitStage(TApplication *app);
extern void initStreamInfo(TApplication *app);
extern void printStreamInfo(TApplication *app, const J2DOrthoGraph *graph);

// FPS
extern void updateFPS(TMarDirector *);

// LOADING SCREEN
extern void initLoadingScreen();

extern void initUnlockedSettings(TApplication *);
extern void checkForCompletionAwards(TApplication *);
extern void updateUnlockedSettings(TApplication *);
extern void drawUnlockedSettings(TApplication *, const J2DOrthoGraph *);

extern void resetPlayerDatas(TMarDirector *application);

// SAVE FILE
extern void initAutoSaveIcon(TApplication *);
extern void updateAutoSaveIcon(TApplication *);
extern void drawAutoSaveIcon(TApplication *, const J2DOrthoGraph *);

// STAGES
extern void initAreaInfo();

extern void patches_staticResetter(TMarDirector *);

// TOOLBOX

extern void initializeTaskBuffers();
extern void processCurrentTask(TApplication *app);

// ================================= //

extern "C" void __cxa_pure_virtual();

static void initLib() {
    makeExtendedObjDataTable();
    initLoadingScreen();
    initExtendedPlayerAnims();
    initAreaInfo();

    initializeTaskBuffers();

    // Toolbox Listener
    Game::addLoopCallback(processCurrentTask);

    // SETTINGS
    {
        sSettingsGroup.addSetting(&gBugFixesSetting);
        sSettingsGroup.addSetting(&gExploitFixesSetting);
        sSettingsGroup.addSetting(&gCollisionFixesSetting);
        sSettingsGroup.addSetting(&gViewportSetting);
        sSettingsGroup.addSetting(&gAspectRatioSetting);
        sSettingsGroup.addSetting(&gFPSSetting);
        sSettingsGroup.addSetting(&gCameraInvertXSetting);
        sSettingsGroup.addSetting(&gCameraInvertYSetting);
        sSettingsGroup.addSetting(&gSavePromptSetting);
        sSettingsGroup.addSetting(&gFirstBootSetting);

        auto &saveInfo        = sSettingsGroup.getSaveInfo();
        saveInfo.mSaveName    = Settings::getGroupName(sSettingsGroup);
        saveInfo.mBlocks      = 1;
        saveInfo.mGameCode    = 'GMSB';
        saveInfo.mCompany     = 0x3031;
        saveInfo.mBannerFmt   = CARD_BANNER_CI;
        saveInfo.mBannerImage = GetResourceTextureHeader(gSaveBnr);
        saveInfo.mIconFmt     = CARD_ICON_CI;
        saveInfo.mIconSpeed   = CARD_SPEED_SLOW;
        saveInfo.mIconCount   = 2;
        saveInfo.mIconTable   = GetResourceTextureHeader(gSaveIcon);
        saveInfo.mSaveGlobal  = false;

        BetterSMS::registerModule(sBetterSMSInfo);
    }

    //

    // Set up application context handlers
    Application::registerContextCallback(TApplication::CONTEXT_GAME_BOOT, BetterAppContextGameBoot);
    Application::registerContextCallback(TApplication::CONTEXT_GAME_BOOT_LOGO,
                                         BetterAppContextGameBootLogo);
    Application::registerContextCallback(TApplication::CONTEXT_GAME_INTRO,
                                         BetterAppContextGameBootIntro);
    Application::registerContextCallback(TApplication::CONTEXT_DIRECT_STAGE,
                                         BetterAppContextDirectStage);
    Application::registerContextCallback(TApplication::CONTEXT_DIRECT_MOVIE,
                                         BetterAppContextDirectMovie);
    Application::registerContextCallback(TApplication::CONTEXT_DIRECT_SHINE_SELECT,
                                         BetterAppContextDirectShineSelect);
    Application::registerContextCallback(TApplication::CONTEXT_DIRECT_LEVEL_SELECT,
                                         BetterAppContextDirectLevelSelect);
    Application::registerContextCallback(CONTEXT_DIRECT_SETTINGS_MENU, BetterAppContextDirectSettingsMenu);

    //// AUTO SAVE
    Game::addBootCallback(initAutoSaveIcon);
    Game::addLoopCallback(updateAutoSaveIcon);
    Game::addPostDrawCallback(drawAutoSaveIcon);

    //// GAME
    Game::addBootCallback(extendLightEffectToShineCount);

#if BETTER_SMS_EXTRA_COLLISION
    // Set up player map collisions
    Player::addInitCallback(initializeWarpCallback);
    Player::addUpdateCallback(processWarpCallback);
    Player::registerCollisionHandler(3000, decHealth);
    Player::registerCollisionHandler(3001, incHealth);
    Player::registerCollisionHandler(3010, elecPlayer);
    Player::registerCollisionHandler(3011, burnPlayer);
    Player::registerCollisionHandler(3030, changeNozzleSpray);
    Player::registerCollisionHandler(3031, changeNozzleHover);
    Player::registerCollisionHandler(3032, changeNozzleTurbo);
    Player::registerCollisionHandler(3033, changeNozzleRocket);
    Player::registerCollisionHandler(3035, changeNozzleSprayOnTouch);
    Player::registerCollisionHandler(3036, changeNozzleHoverOnTouch);
    Player::registerCollisionHandler(3037, changeNozzleTurboOnTouch);
    Player::registerCollisionHandler(3038, changeNozzleRocketOnTouch);
    Player::registerCollisionHandler(3040, setGravityCol);
    Player::registerCollisionHandler(3041, antiGravityCol);
    Player::registerCollisionHandler(3042, boostPadCol);
    Player::registerCollisionHandler(3060, instantWarpHandler);
    Player::registerCollisionHandler(3061, screenWipeWarpHandler);
    Player::registerCollisionHandler(3062, effectWarpHandler);
    Player::registerCollisionHandler(3063, instantScreenWipeWarpHandler);
    Player::registerCollisionHandler(3064, portalWarpHandler);
    Player::registerCollisionHandler(3065, portalFreeWarpHandler);
#endif

    //// PLAYER STATE
    Player::addUpdateCallback(updateCollisionContext);
    Player::addUpdateCallback(updateClimbContext);
    Player::addUpdateCallback(checkForForceDropOnDeadActor);

    //// YOSHI
    Player::addUpdateCallback(checkForYoshiDeath);
    Player::addUpdateCallback(forceValidRidingAnimation);

    //// DEBUG
    Player::addUpdateCallback(checkDebugMode);
    Player::addUpdateCallback(updateDeadTriggerState);
    Player::registerStateMachine(XYZState, updateDebugMode);

    Debug::addUpdateCallback(updateFluddNozzle);

    Debug::addDrawCallback(drawMonitor);
    Stage::addExitCallback(resetMonitor);

    Debug::addInitCallback(initFPSMonitor);
    Debug::addUpdateCallback(updateFPSMonitor);
    Debug::addDrawCallback(drawFPSMonitor);

    Debug::addInitCallback(initGameStateMonitor);
    Debug::addUpdateCallback(updateGameStateMonitor);
    Debug::addDrawCallback(drawGameStateMonitor);

    Debug::addInitCallback(initDebugStateMonitor);
    Debug::addUpdateCallback(updateDebugStateMonitor);
    Debug::addDrawCallback(drawDebugStateMonitor);

    Stage::addInitCallback(resetDebugState);

    // Music
    Game::addChangeCallback(stopMusicOnExitStage);
    Debug::addInitCallback(initStreamInfo);
    Debug::addDrawCallback(printStreamInfo);

    Objects::registerObjectAsMapObj("GenericRailObj", &generic_railobj_data,
                                    TGenericRailObj::instantiate);
    Objects::registerObjectAsMisc("SimpleFog", TSimpleFog::instantiate);
    Objects::registerObjectAsMisc("ParticleBox", TParticleBox::instantiate);
    Objects::registerObjectAsMisc("SoundBox", TSoundBox::instantiate);

    Game::addBootCallback(initDebugCallbacks);

    Stage::addInitCallback(resetPlayerDatas);

    Stage::addInitCallback(updateFPS);
    Stage::addUpdateCallback(updateFPS);

    // SETTINGS
    Game::addBootCallback(initUnlockedSettings);
    Game::addLoopCallback(updateUnlockedSettings);
    Game::addPostDrawCallback(drawUnlockedSettings);

    // PATCHES
    Stage::addInitCallback(patches_staticResetter);
}

static void destroyLib() {}

KURIBO_MODULE_BEGIN(BETTER_SMS_MODULE_NAME, BETTER_SMS_AUTHOR_NAME, BETTER_SMS_VERSION_TAG) {
    KURIBO_EXECUTE_ON_LOAD {
        initLib();

        // Generate exports

        /* GAME CRITICAL */
        KURIBO_EXPORT_AS(BetterSMS::Game::getMaxShines, "getMaxShines__Q29BetterSMS4GameFv");
        KURIBO_EXPORT_AS(BetterSMS::Game::setMaxShines, "setMaxShines__Q29BetterSMS4GameFUl");

        /* MODULE */
        KURIBO_EXPORT_AS(BetterSMS::getModuleInfo, "getModuleInfo__9BetterSMSFPCc");
        KURIBO_EXPORT_AS(BetterSMS::isModuleRegistered, "isModuleRegistered__9BetterSMSFPCc");
        KURIBO_EXPORT_AS(BetterSMS::registerModule,
                         "registerModule__9BetterSMSFRCQ29BetterSMS10ModuleInfo");
        KURIBO_EXPORT_AS(BetterSMS::isGameEmulated, "isGameEmulated__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::isMusicBeingStreamed, "isMusicBeingStreamed__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::isMusicStreamingAllowed,
                         "isMusicStreamingAllowed__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::isDebugMode, "isDebugMode__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::setDebugMode, "setDebugMode__9BetterSMSFb");
        KURIBO_EXPORT_AS(BetterSMS::getScreenRenderWidth, "getScreenRenderWidth__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::getScreenOrthoWidth, "getScreenOrthoWidth__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::getScreenToFullScreenRatio,
                         "getScreenToFullScreenRatio__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::getScreenRatioAdjustX, "getScreenRatioAdjustX__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::getFrameRate, "getFrameRate__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::getBugFixesSetting, "getBugFixesSetting__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::getExploitFixesSetting, "getExploitFixesSetting__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::getCollisionFixesSetting,
                         "getCollisionFixesSetting__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::triggerAutoSave, "triggerAutoSave__9BetterSMSFv");

        /* SETTINGS */
        KURIBO_EXPORT_AS(BetterSMS::areBugsPatched, "areBugsPatched__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::areExploitsPatched, "areExploitsPatched__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::isCollisionRepaired, "isCollisionRepaired__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::isCameraInvertedX, "isCameraInvertedX__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::isCameraInvertedY, "isCameraInvertedY__9BetterSMSFv");

        // Hack for circular reference
        KURIBO_EXPORT_AS(
            BetterSMS::Settings::getGroupName,
            "getGroupName__Q29BetterSMS8SettingsFRCQ39BetterSMS8Settings13SettingsGroup");
        KURIBO_EXPORT_AS(BetterSMS::Settings::mountCard, "mountCard__Q29BetterSMS8SettingsFv");
        KURIBO_EXPORT_AS(BetterSMS::Settings::unmountCard, "unmountCard__Q29BetterSMS8SettingsFv");
        KURIBO_EXPORT_AS(
            BetterSMS::Settings::loadSettingsGroup,
            "loadSettingsGroup__Q29BetterSMS8SettingsFRQ39BetterSMS8Settings13SettingsGroup");
        KURIBO_EXPORT_AS(
            BetterSMS::Settings::saveSettingsGroup,
            "saveSettingsGroup__Q29BetterSMS8SettingsFRQ39BetterSMS8Settings13SettingsGroup");
        KURIBO_EXPORT_AS(BetterSMS::Settings::loadAllSettings,
                         "loadAllSettings__Q29BetterSMS8SettingsFv");
        KURIBO_EXPORT_AS(BetterSMS::Settings::saveAllSettings,
                         "saveAllSettings__Q29BetterSMS8SettingsFv");

        /* APPLICATION */
        KURIBO_EXPORT_AS(
            BetterSMS::Application::registerContextCallback,
            "registerContextCallback__Q29BetterSMS11ApplicationFUcPFP12TApplication_b");
        KURIBO_EXPORT_AS(BetterSMS::Application::setIntroStage,
                         "setIntroStage__Q29BetterSMS11ApplicationFUcUc");
        KURIBO_EXPORT_AS(BetterSMS::Application::isFirstBoot,
                         "isFirstBoot__Q29BetterSMS11ApplicationFv");
        KURIBO_EXPORT_AS(BetterSMS::Application::showSettingsOnFirstBoot,
                         "showSettingsOnFirstBoot__Q29BetterSMS11ApplicationFb");

        /* BMG */
        KURIBO_EXPORT_AS(
            BetterSMS::BMG::registerBMGCommandCallback,
            "registerBMGCommandCallback__Q29BetterSMS3BMGFUcPFPCUcCUcP12TFlagManager_PCc");

        /* LOGGING */
        KURIBO_EXPORT_AS(BetterSMS::Console::log, "log__Q29BetterSMS7ConsoleFPCce");
        KURIBO_EXPORT_AS(BetterSMS::Console::emulatorLog, "emulatorLog__Q29BetterSMS7ConsoleFPCce");
        KURIBO_EXPORT_AS(BetterSMS::Console::hardwareLog, "hardwareLog__Q29BetterSMS7ConsoleFPCce");
        KURIBO_EXPORT_AS(BetterSMS::Console::debugLog, "debugLog__Q29BetterSMS7ConsoleFPCce");

        /* DEBUG */
        KURIBO_EXPORT_AS(BetterSMS::Debug::addInitCallback,
                         "addInitCallback,__Q29BetterSMS5DebugFPFP12TApplication_v");
        KURIBO_EXPORT_AS(BetterSMS::Debug::addUpdateCallback,
                         "addUpdateCallback__Q29BetterSMS5DebugFPFP12TApplication_v");
        KURIBO_EXPORT_AS(
            BetterSMS::Debug::addDrawCallback,
            "addDrawCallback__Q29BetterSMS5DebugFPFP12TApplicationPC13J2DOrthoGraph_v");

        /* MEMORY */
        KURIBO_EXPORT_AS(BetterSMS::Memory::malloc, "malloc__Q29BetterSMS6MemoryFUlUl");
        KURIBO_EXPORT_AS(BetterSMS::Memory::calloc, "calloc__Q29BetterSMS6MemoryFUlUl");
        KURIBO_EXPORT_AS(BetterSMS::Memory::hmalloc, "hmalloc__Q29BetterSMS6MemoryFP7JKRHeapUlUl");
        KURIBO_EXPORT_AS(BetterSMS::Memory::hcalloc, "hcalloc__Q29BetterSMS6MemoryFP7JKRHeapUlUl");
        KURIBO_EXPORT_AS(BetterSMS::Memory::free, "free__Q29BetterSMS6MemoryFPCv");
        KURIBO_EXPORT_AS(BetterSMS::PowerPC::getBranchDest,
                         "getBranchDest__Q29BetterSMS7PowerPCFPUl");
        KURIBO_EXPORT_AS(BetterSMS::PowerPC::writeU8, "writeU8__Q29BetterSMS7PowerPCFPUcUc");
        KURIBO_EXPORT_AS(BetterSMS::PowerPC::writeU16, "writeU16__Q29BetterSMS7PowerPCFPUsUs");
        KURIBO_EXPORT_AS(BetterSMS::PowerPC::writeU32, "writeU32__Q29BetterSMS7PowerPCFPUlUl");

        /* LOADING */
        KURIBO_EXPORT_AS(BetterSMS::Loading::setLoading, "setLoading__Q29BetterSMS7LoadingFb");
        KURIBO_EXPORT_AS(BetterSMS::Loading::setLoadingIcon,
                         "setLoadingIcon__Q29BetterSMS7LoadingFPPC7ResTIMGUl");
        KURIBO_EXPORT_AS(BetterSMS::Loading::setLayout,
                         "setLayout__Q29BetterSMS7LoadingFP9J2DScreen");
        KURIBO_EXPORT_AS(BetterSMS::Loading::setFrameRate, "setFrameRate__Q29BetterSMS7LoadingFf");

        /* PLAYER */
        KURIBO_EXPORT_AS(BetterSMS::Player::getRegisteredData,
                         "getRegisteredData__Q29BetterSMS6PlayerFP6TMarioPCc");
        KURIBO_EXPORT_AS(BetterSMS::Player::getData, "getData__Q29BetterSMS6PlayerFP6TMario");
        KURIBO_EXPORT_AS(BetterSMS::Player::registerData,
                         "registerData__Q29BetterSMS6PlayerFP6TMarioPCcPv");
        KURIBO_EXPORT_AS(BetterSMS::Player::deregisterData,
                         "deregisterData__Q29BetterSMS6PlayerFP6TMarioPCc");
        KURIBO_EXPORT_AS(BetterSMS::Player::isAnimationValid,
                         "isAnimationValid__Q29BetterSMS6PlayerFUs");
        KURIBO_EXPORT_AS(BetterSMS::Player::setAnimationData,
                         "setAnimationData__Q29BetterSMS6PlayerFUsPbPbPUcPUc");
        KURIBO_EXPORT_AS(BetterSMS::Player::addAnimationData,
                         "addAnimationData__Q29BetterSMS6PlayerFPCcbbUcUc");
        KURIBO_EXPORT_AS(BetterSMS::Player::addAnimationDataEx,
                         "addAnimationDataEx__Q29BetterSMS6PlayerFUsPCcbbUcUc");
        KURIBO_EXPORT_AS(BetterSMS::Player::addInitCallback,
                         "addInitCallback__Q29BetterSMS6PlayerFPFP6TMariob_v");
        KURIBO_EXPORT_AS(BetterSMS::Player::addLoadAfterCallback,
                         "addLoadAfterCallback__Q29BetterSMS6PlayerFPFP6TMario_v");
        KURIBO_EXPORT_AS(BetterSMS::Player::addUpdateCallback,
                         "addUpdateCallback__Q29BetterSMS6PlayerFPFP6TMariob_v");
        KURIBO_EXPORT_AS(BetterSMS::Player::registerStateMachine,
                         "registerStateMachine__Q29BetterSMS6PlayerFUlPFP6TMario_b");
        KURIBO_EXPORT_AS(
            BetterSMS::Player::registerCollisionHandler,
            "registerCollisionHandler__Q29BetterSMS6PlayerFUsPFP6TMarioPC12TBGCheckDataUl_v");
        KURIBO_EXPORT_AS(BetterSMS::Player::warpToCollisionFace,
                         "warpToCollisionFace__Q29BetterSMS6PlayerFv");
        KURIBO_EXPORT_AS(BetterSMS::Player::warpToPoint, "warpToPoint__Q29BetterSMS6PlayerFv");
        KURIBO_EXPORT_AS(
            BetterSMS::Player::rotateRelativeToCamera,
            "rotateRelativeToCamera__Q29BetterSMS6PlayerFP6TMarioP15CPolarSubCamera4Vec2f");

        /* MUSIC */
        KURIBO_EXPORT_AS(BetterSMS::Music::queueSong, "queueSong__Q29BetterSMS5MusicFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Music::playSong, "playSong__Q29BetterSMS5MusicFv");
        KURIBO_EXPORT_AS(BetterSMS::Music::pauseSong, "pauseSong__Q29BetterSMS5MusicFf");
        KURIBO_EXPORT_AS(BetterSMS::Music::stopSong, "stopSong__Q29BetterSMS5MusicFf");
        KURIBO_EXPORT_AS(BetterSMS::Music::skipSong, "skipSong__Q29BetterSMS5MusicFf");
        KURIBO_EXPORT_AS(BetterSMS::Music::setVolume, "setVolume__Q29BetterSMS5MusicFUcUc");
        KURIBO_EXPORT_AS(BetterSMS::Music::setVolumeFade, "setVolumeFade__Q29BetterSMS5MusicFUcf");
        KURIBO_EXPORT_AS(BetterSMS::Music::setMaxVolume, "setMaxVolume__Q29BetterSMS5MusicFUc");
        KURIBO_EXPORT_AS(BetterSMS::Music::setLoopPoint, "setLoopPoint__Q29BetterSMS5MusicFff");

/* OBJECTS */
#if BETTER_SMS_EXTRA_OBJECTS
        KURIBO_EXPORT_AS(
            BetterSMS::Objects::registerObjectAsMapObj,
            "registerObjectAsMapObj__Q29BetterSMS7ObjectsFPCcP7ObjDataPFv_PQ26JDrama8TNameRef");
        KURIBO_EXPORT_AS(
            BetterSMS::Objects::registerObjectAsEnemy,
            "registerObjectAsEnemy__Q29BetterSMS7ObjectsFPCcP7ObjDataPFv_PQ26JDrama8TNameRef");
        KURIBO_EXPORT_AS(BetterSMS::Objects::registerObjectAsMisc,
                         "registerObjectAsMisc__Q29BetterSMS7ObjectsFPCcPFv_PQ26JDrama8TNameRef");
        KURIBO_EXPORT_AS(BetterSMS::Objects::getRemainingCapacity,
                         "getRemainingCapacity__Q29BetterSMS7ObjectsFv");
        KURIBO_EXPORT_AS(
            BetterSMS::Objects::registerObjectCollideInteractor,
            "registerObjectCollideInteractor__Q29BetterSMS7ObjectsFUlPFP9THitActorP6TMario_v");
        KURIBO_EXPORT_AS(
            BetterSMS::Objects::registerObjectGrabInteractor,
            "registerObjectGrabInteractor__Q29BetterSMS7ObjectsFUlPFP9THitActorP6TMario_v");
#endif

        /* GAME */
        KURIBO_EXPORT_AS(BetterSMS::Game::addInitCallback,
                         "addInitCallback__Q29BetterSMS4GameFPFP12TApplication_v");
        KURIBO_EXPORT_AS(BetterSMS::Game::addBootCallback,
                         "addBootCallback__Q29BetterSMS4GameFPFP12TApplication_v");
        KURIBO_EXPORT_AS(BetterSMS::Game::addLoopCallback,
                         "addLoopCallback__Q29BetterSMS4GameFPFP12TApplication_v");
        KURIBO_EXPORT_AS(
            BetterSMS::Game::addPostDrawCallback,
            "addPostDrawCallback__Q29BetterSMS4GameFPFP12TApplicationPC13J2DOrthoGraph_v");
        KURIBO_EXPORT_AS(BetterSMS::Game::addChangeCallback,
                         "addChangeCallback__Q29BetterSMS4GameFPFP12TApplication_v");

        /* STAGE */
        KURIBO_EXPORT_AS(BetterSMS::Stage::registerStageInfo,
                         "registerStageInfo__Q29BetterSMS5StageFUcPQ39BetterSMS5Stage8AreaInfo");
        KURIBO_EXPORT_AS(
            BetterSMS::Stage::registerExStageInfo,
            "registerExStageInfo__Q29BetterSMS5StageFUcPQ39BetterSMS5Stage10ExAreaInfo");
        KURIBO_EXPORT_AS(BetterSMS::Stage::setNextStageHandler,
                         "setNextStageHandler__Q29BetterSMS5StageFPFP12TMarDirector_v");
        KURIBO_EXPORT_AS(BetterSMS::Stage::getStageConfiguration,
                         "getStageConfiguration__Q29BetterSMS5StageFv");
        KURIBO_EXPORT_AS(BetterSMS::Stage::addInitCallback,
                         "addInitCallback__Q29BetterSMS5StageFPFP12TMarDirector_v");
        KURIBO_EXPORT_AS(BetterSMS::Stage::addUpdateCallback,
                         "addUpdateCallback__Q29BetterSMS5StageFPFP12TMarDirector_v");
        KURIBO_EXPORT_AS(
            BetterSMS::Stage::addDraw2DCallback,
            "addDraw2DCallback__Q29BetterSMS5StageFPFP12TMarDirectorPC13J2DOrthoGraph_v");
        KURIBO_EXPORT_AS(
            BetterSMS::Stage::addExitCallback,
            "addExitCallback__Q29BetterSMS5StageFPFP12TApplication_v");
        KURIBO_EXPORT_AS(BetterSMS::Stage::getStageName, "getStageName__Q29BetterSMS5StageFUcUc");
        KURIBO_EXPORT_AS(BetterSMS::Stage::isExStage, "isExStage__Q29BetterSMS5StageFUcUc");

        /* TIME */
        KURIBO_EXPORT_AS(BetterSMS::Time::buildDate, "buildDate__Q29BetterSMS4TimeFv");
        KURIBO_EXPORT_AS(BetterSMS::Time::buildTime, "buildTime__Q29BetterSMS4TimeFv");
        KURIBO_EXPORT_AS(BetterSMS::Time::getCalendar,
                         "getCalendar__Q29BetterSMS4TimeFR14OSCalendarTime");
        KURIBO_EXPORT_AS(BetterSMS::Time::calendarToDate,
                         "calendarToDate__Q29BetterSMS4TimeFPcRC14OSCalendarTime");
        KURIBO_EXPORT_AS(BetterSMS::Time::calendarToTime,
                         "calendarToTime__Q29BetterSMS4TimeFPcRC14OSCalendarTime");
        KURIBO_EXPORT_AS(BetterSMS::Time::ostime, "ostime__Q29BetterSMS4TimeFv");

        /* CTYPE */
        KURIBO_EXPORT(isxdigit);
        KURIBO_EXPORT(isupper);
        KURIBO_EXPORT(isspace);
        KURIBO_EXPORT(ispunct);
        KURIBO_EXPORT(isprint);
        KURIBO_EXPORT(islower);
        KURIBO_EXPORT(isgraph);
        KURIBO_EXPORT(isdigit);
        KURIBO_EXPORT(iscntrl);
        KURIBO_EXPORT(isalpha);
        KURIBO_EXPORT(isalnum);

        /* MATH */
        KURIBO_EXPORT(log);
        KURIBO_EXPORT(sqrtf);

        /* STRLIB */
        KURIBO_EXPORT(reverse);
        KURIBO_EXPORT(itoa);

        /* CXA VIRTUAL */
        KURIBO_EXPORT(__cxa_pure_virtual);
    }
    KURIBO_EXECUTE_ON_UNLOAD { destroyLib(); }
}
KURIBO_MODULE_END()

extern "C" void __cxa_pure_virtual() { SMS_ASM_BLOCK("trap \r\n"); }