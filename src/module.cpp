#include <Dolphin/OS.h>
#include <SMS/macros.h>

#include "collision/warp.hxx"

// BetterSMS API
#include "application.hxx"
#include "bmg.hxx"
#include "cstd/ctype.h"
#include "cstd/stdlib.h"
#include "debug.hxx"
#include "game.hxx"
#include "loading.hxx"
#include "logging.hxx"
#include "memory.hxx"
#include "module.hxx"
#include "music.hxx"
#include "object.hxx"
#include "objects/generic.hxx"
#include "player.hxx"
#include "p_settings.hxx"
#include "settings.hxx"
#include "stage.hxx"
#include "time.hxx"

// SETTINGS //

// Sunshine settings
static Settings::SettingsGroup sBaseSettingsGroup("Super Mario Sunshine", 1, 0, Settings::Priority::CORE);
RumbleSetting gRumbleSetting("Controller Rumble");
SoundSetting gSoundSetting("Sound Mode");
SubtitleSetting gSubtitleSetting("Movie Subtitles");
//

// BetterSMS settings
static Settings::SettingsGroup sSettingsGroup("Better Sunshine Engine", 1, 0,
                                              Settings::Priority::CORE);

BugsSetting gBugFixesSetting("Bug & Exploit Fixes");

AspectRatioSetting gAspectRatioSetting("Aspect Ratio");
FPSSetting gFPSSetting("Frame Rate");

static bool sCameraInvertX;
static bool sCameraInvertY;
Settings::SwitchSetting gCameraInvertXSetting("Invert Camera X", &sCameraInvertX);
Settings::SwitchSetting gCameraInvertYSetting("Invert Camera Y", &sCameraInvertY);
//

// mario.cpp
extern "C" s16 mario_shadowCrashPatch();

// shine.cpp
extern "C" void shine_animationFreezeCheck();
extern "C" void shine_thinkCloseCamera();

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

extern void initMarioXYZMode(TApplication *);
extern void updateMarioXYZMode(TApplication *);
extern void updateFluddNozzle(TApplication *);

extern void drawMonitor(TApplication *, const J2DOrthoGraph *);
extern void resetMonitor(TApplication *);

extern void initFPSMonitor(TApplication *);
extern void updateFPSMonitor(TApplication *);
extern void drawFPSMonitor(TApplication *, const J2DOrthoGraph *);

extern void initStateMonitor(TApplication *);
extern void updateStateMonitor(TApplication *);
extern void drawStateMonitor(TApplication *, const J2DOrthoGraph *);

// STAGE CONFIG
extern void loadStageConfig(TMarDirector *);
extern void resetGlobalValues(TApplication *);

// PLAYER CONFIG
extern void initMario(TMario *, bool);
extern void resetPlayerDatas(TApplication *);

// PLAYER WARP
extern void processWarp(TMario *, bool);
extern void updateDebugCollision(TMario *, bool);

// PLAYER MOVES
extern u32 CrouchState;
extern void checkForCrouch(TMario *, bool);
extern bool processCrouch(TMario *);

extern u32 MultiJumpState;
extern void checkForMultiJump(TMario *, bool);
extern bool processMultiJump(TMario *);

// PLAYER COLLISION
extern void decHealth(TMario *player, const TBGCheckData *data, u32 flags);
extern void incHealth(TMario *player, const TBGCheckData *data, u32 flags);
extern void elecPlayer(TMario *player, const TBGCheckData *data, u32 flags);
extern void burnPlayer(TMario *player, const TBGCheckData *data, u32 flags);
extern void changeNozzleSpray(TMario *player, const TBGCheckData *data, u32 flags);
extern void changeNozzleHover(TMario *player, const TBGCheckData *data, u32 flags);
extern void changeNozzleTurbo(TMario *player, const TBGCheckData *data, u32 flags);
extern void changeNozzleRocket(TMario *player, const TBGCheckData *data, u32 flags);
extern void setGravityCol(TMario *player, const TBGCheckData *data, u32 flags);
extern void antiGravityCol(TMario *player, const TBGCheckData *data, u32 flags);
extern void boostPadCol(TMario *player, const TBGCheckData *data, u32 flags);
extern void instantWarpHandler(TMario *player, const TBGCheckData *data, u32 flags);
extern void screenWipeWarpHandler(TMario *player, const TBGCheckData *data, u32 flags);
extern void effectWarpHandler(TMario *player, const TBGCheckData *data, u32 flags);
extern void portalWarpHandler(TMario *player, const TBGCheckData *data, u32 flags);
extern void portalFreeWarpHandler(TMario *player, const TBGCheckData *data, u32 flags);

// FLUDD
extern void initTurboMaxCapacity(TMario *player, bool isMario);
extern void updateTurboFrameEmit(TMario *player, bool isMario);

// YOSHI
extern void checkForYoshiDeath(TMario *player, bool isMario);
extern void forceValidRidingAnimation(TMario *player, bool isMario);

// MUSIC
extern void initStreamInfo(TApplication *app);
extern void printStreamInfo(TApplication *app, const J2DOrthoGraph *graph);

// FPS
extern void updateFPS(TMarDirector *);

// LOADING SCREEN
extern void initLoadingScreen();

// SETTINGS
extern void initAllSettings(TApplication *);

extern void initUnlockedSettings(TApplication *);
extern void updateUnlockedSettings(TMarDirector *);
extern void checkForCompletionAwards(TApplication *);
extern void drawUnlockedSettings(TApplication *, const J2DOrthoGraph *);

static void initLib() {

#define STRINGIFY_CONFIG(c) " " #c " = " SMS_STRINGIZE(c) "\n"

    // clang-format off
    Console::log("==== BetterSunshineEngine Config ====\n"
                 STRINGIFY_CONFIG(BETTER_SMS_CRASHFIXES)
                 STRINGIFY_CONFIG(BETTER_SMS_BUGFIXES)
                 STRINGIFY_CONFIG(BETTER_SMS_NO_DOWNWARP)
                 STRINGIFY_CONFIG(BETTER_SMS_YOSHI_SAVE_NOZZLES)
                 STRINGIFY_CONFIG(BETTER_SMS_CUSTOM_MUSIC)
                 STRINGIFY_CONFIG(BETTER_SMS_EXTRA_SHINES)
                 STRINGIFY_CONFIG(BETTER_SMS_EXTRA_OBJECTS)
                 STRINGIFY_CONFIG(BETTER_SMS_EXTRA_COLLISION)
                 STRINGIFY_CONFIG(BETTER_SMS_EXTRA_BMG_COMMANDS)
                 STRINGIFY_CONFIG(BETTER_SMS_EXTENDED_RENDER_DISTANCE)
                 STRINGIFY_CONFIG(BETTER_SMS_SWAP_LZ_BUTTONS)
                 STRINGIFY_CONFIG(BETTER_SMS_LONG_JUMP)
                 STRINGIFY_CONFIG(BETTER_SMS_MULTI_JUMP)
                 STRINGIFY_CONFIG(BETTER_SMS_HOVER_BURST)
                 STRINGIFY_CONFIG(BETTER_SMS_HOVER_SLIDE)
                 STRINGIFY_CONFIG(BETTER_SMS_ROCKET_DIVE)
                 STRINGIFY_CONFIG(BETTER_SMS_GREEN_YOSHI)
                 STRINGIFY_CONFIG(BETTER_SMS_UNDERWATER_FRUIT)
                 STRINGIFY_CONFIG(BETTER_SMS_SLOT_B_SUPPORT)
                 STRINGIFY_CONFIG(BETTER_SMS_SHADOW_MARIO_HEALTHBAR)
                 STRINGIFY_CONFIG(BETTER_SMS_DYNAMIC_FALL_DAMAGE)
                 STRINGIFY_CONFIG(BETTER_SMS_EXCEPTION_HANDLER)
                 "=====================================\n");
    // clang-format on

#undef STRINGIFY_CONFIG

    makeExtendedObjDataTable();
    initLoadingScreen();

    // SETTINGS
    sBaseSettingsGroup.addSetting(&gRumbleSetting);
    sBaseSettingsGroup.addSetting(&gSoundSetting);
    sBaseSettingsGroup.addSetting(&gSubtitleSetting);
    Settings::registerGroup("Super Mario Sunshine", &sBaseSettingsGroup);

    sSettingsGroup.addSetting(&gBugFixesSetting);
    sSettingsGroup.addSetting(&gAspectRatioSetting);
    sSettingsGroup.addSetting(&gFPSSetting);
    sSettingsGroup.addSetting(&gCameraInvertXSetting);
    sSettingsGroup.addSetting(&gCameraInvertYSetting);
    {
        auto &saveInfo        = sSettingsGroup.getSaveInfo();
        saveInfo.mSaveName    = sSettingsGroup.getName();
        saveInfo.mBlocks      = 1;
        saveInfo.mGameCode    = 'GMSB';
        // saveInfo.mCompany     = '01';
        saveInfo.mCompany     = 0x3031;
        saveInfo.mBannerFmt   = CARD_BANNER_CI;
        saveInfo.mBannerImage = GetResourceTextureHeader(gSaveBnr);
        saveInfo.mIconFmt     = CARD_ICON_CI;
        saveInfo.mIconSpeed   = CARD_SPEED_SLOW;
        saveInfo.mIconCount   = 2;
        saveInfo.mIconTable   = GetResourceTextureHeader(gSaveIcon);
        saveInfo.mSaveGlobal  = true;
    }
    Settings::registerGroup("Better Sunshine Engine", &sSettingsGroup);

    //

    // Set up application context handlers
    Application::registerContextCallback(TApplication::CONTEXT_GAME_BOOT,
                                         BetterAppContextGameBoot);
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
    Application::registerContextCallback(10, BetterAppContextDirectSettingsMenu);

    // Set up stage config handlers
    Stage::registerInitCallback("__init_config", loadStageConfig);
    Stage::registerExitCallback("__reset_globals", resetGlobalValues);

    // Set up player params
    Player::registerInitProcess("__init_mario", initMario);
    Stage::registerExitCallback("__destroy_mario", resetPlayerDatas);

    Player::registerUpdateProcess("__update_mario_crouch", checkForCrouch);
    Player::registerStateMachine(CrouchState, processCrouch);

    Player::registerUpdateProcess("__update_mario_multijump", checkForMultiJump);
    Player::registerStateMachine(MultiJumpState, processMultiJump);

    Player::registerUpdateProcess("__debug_warp_collision", updateDebugCollision);

    // Set up player map collisions
    Player::registerCollisionHandler(3000, decHealth);
    Player::registerCollisionHandler(3001, incHealth);
    Player::registerCollisionHandler(3010, elecPlayer);
    Player::registerCollisionHandler(3011, burnPlayer);
    Player::registerCollisionHandler(3030, changeNozzleSpray);
    Player::registerCollisionHandler(3031, changeNozzleHover);
    Player::registerCollisionHandler(3032, changeNozzleTurbo);
    Player::registerCollisionHandler(3033, changeNozzleRocket);
    Player::registerCollisionHandler(3040, setGravityCol);
    Player::registerCollisionHandler(3041, antiGravityCol);
    Player::registerCollisionHandler(3042, boostPadCol);
    Player::registerCollisionHandler(3060, instantWarpHandler);
    Player::registerCollisionHandler(3061, screenWipeWarpHandler);
    Player::registerCollisionHandler(3062, effectWarpHandler);
    Player::registerCollisionHandler(3063, portalWarpHandler);
    Player::registerCollisionHandler(3064, portalFreeWarpHandler);

    // FLUDD
    Player::registerInitProcess("__init_turbo_max", initTurboMaxCapacity);
    Player::registerUpdateProcess("__update_turbo_usage", updateTurboFrameEmit);

    // YOSHI
    Player::registerUpdateProcess("__update_yoshi_swim", checkForYoshiDeath);
    Player::registerUpdateProcess("__update_yoshi_riding", forceValidRidingAnimation);

    // DEBUG
    Debug::registerInitCallback("__init_debug_xyz", initMarioXYZMode);
    Debug::registerUpdateCallback("__update_debug_xyz", updateMarioXYZMode);
    Debug::registerUpdateCallback("__update_fludd_nozzle", updateFluddNozzle);

    Debug::registerDrawCallback("__draw_memory_usage", drawMonitor);
    Stage::registerExitCallback("__reset_memory_usage", resetMonitor);

    Debug::registerInitCallback("__init_fps_counter", initFPSMonitor);
    Debug::registerUpdateCallback("__update_fps_counter", updateFPSMonitor);
    Debug::registerDrawCallback("__draw_fps_counter", drawFPSMonitor);

    Debug::registerInitCallback("__init_state_counter", initStateMonitor);
    Debug::registerUpdateCallback("__update_state_counter", updateStateMonitor);
    Debug::registerDrawCallback("__draw_state_counter", drawStateMonitor);

    /*Debug::registerInitCallback("__init_music_state", initStreamInfo);
    Debug::registerDrawCallback("__draw_music_state", printStreamInfo);*/

    Objects::registerObjectAsMapObj("GenericRailObj", &generic_railobj_data,
                                    TGenericRailObj::instantiate);

    Game::registerOnBootCallback("__init_debug_handles", initDebugCallbacks);

    //Game::registerOnBootCallback("__init_fps", updateFPSBoot);
    Stage::registerInitCallback("__init_fps", updateFPS);
    Stage::registerUpdateCallback("__update_fps", updateFPS);

    // SETTINGS
    Game::registerOnBootCallback("__load_settings", initAllSettings);
    Game::registerOnBootCallback("__init_setting_notifs", initUnlockedSettings);
    Stage::registerUpdateCallback("__update_setting_notifs", updateUnlockedSettings);
    Debug::registerUpdateCallback("__check_awards", checkForCompletionAwards);
    Game::registerOnPostDrawCallback("__draw_setting_notifs", drawUnlockedSettings);
}

static void destroyLib() {
    Console::log("-- Destroying Module --\n");

    // Remove debug handlers
    Stage::deregisterInitCallback("__init_debug");
    Stage::deregisterUpdateCallback("__update_debug");
    Stage::deregisterDraw2DCallback("__draw_debug");

    // Remove config handlers
    Stage::deregisterInitCallback("__init_globals");
    Stage::deregisterInitCallback("__init_config");

    // Set up player params
    Player::deregisterInitProcess("__init_mario");

    // Remove loading screen
    Stage::deregisterInitCallback("__init_load_screen");
    Stage::deregisterUpdateCallback("__update_load_screen");
}

KURIBO_MODULE_BEGIN(BETTER_SMS_MODULE_NAME, BETTER_SMS_AUTHOR_NAME, BETTER_SMS_VERSION_TAG)
{
    KURIBO_EXECUTE_ON_LOAD {
        initLib();

        // Generate exports

        /* MODULE */
        KURIBO_EXPORT_AS(BetterSMS::isGameEmulated, "isGameEmulated__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::isMusicBeingStreamed, "isMusicBeingStreamed__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::isMusicStreamingAllowed, "isMusicStreamingAllowed__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::isDebugMode, "isDebugMode__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::setDebugMode, "setDebugMode__9BetterSMSFb");
        KURIBO_EXPORT_AS(BetterSMS::getScreenRenderWidth, "getScreenRenderWidth__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::getScreenOrthoWidth, "getScreenOrthoWidth__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::getScreenToFullScreenRatio, "getScreenToFullScreenRatio__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::getScreenRatioAdjustX, "getScreenRatioAdjustX__9BetterSMSFv");
        KURIBO_EXPORT_AS(BetterSMS::getFrameRate, "getFrameRate__9BetterSMSFv");

        /* BMG */
        KURIBO_EXPORT_AS(BetterSMS::BMG::isBMGCommandRegistered, "getFrameRate__Q29BetterSMS3BMGFUc");
        KURIBO_EXPORT_AS(BetterSMS::BMG::registerBMGCommandCallback,
                         "registerBMGCommandCallback__Q29BetterSMS3BMGFUcPFPCUcCUcP12TFlagManager_PCc");
        KURIBO_EXPORT_AS(BetterSMS::BMG::deregisterBMGCommandCallback,
                         "deregisterBMGCommandCallback__Q29BetterSMS3BMGFUc");

        /* LOGGING */
        KURIBO_EXPORT_AS(BetterSMS::Console::log, "log__Q29BetterSMS7ConsoleFPCce");
        KURIBO_EXPORT_AS(BetterSMS::Console::emulatorLog, "emulatorLog__Q29BetterSMS7ConsoleFPCce");
        KURIBO_EXPORT_AS(BetterSMS::Console::hardwareLog, "hardwareLog__Q29BetterSMS7ConsoleFPCce");
        KURIBO_EXPORT_AS(BetterSMS::Console::debugLog, "debugLog__Q29BetterSMS7ConsoleFPCce");

        /* DEBUG */
        KURIBO_EXPORT_AS(BetterSMS::Debug::isInitRegistered, "isInitRegistered__Q29BetterSMS5DebugFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Debug::isUpdateRegistered, "isUpdateRegistered__Q29BetterSMS5DebugFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Debug::registerInitCallback, "registerInitCallback,__Q29BetterSMS5DebugFPCcPFP12TApplication_v");
        KURIBO_EXPORT_AS(BetterSMS::Debug::registerUpdateCallback, "registerUpdateCallback__Q29BetterSMS5DebugFPCcPFP12TApplication_v");
        KURIBO_EXPORT_AS(BetterSMS::Debug::deregisterInitCallback, "deregisterInitCallback__Q29BetterSMS5DebugFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Debug::deregisterUpdateCallback,
                         "deregisterUpdateCallback__Q29BetterSMS5DebugFPCc");

        /* MEMORY */
        KURIBO_EXPORT_AS(BetterSMS::Memory::malloc, "malloc__Q29BetterSMS6MemoryFUlUl");
        KURIBO_EXPORT_AS(BetterSMS::Memory::calloc, "calloc__Q29BetterSMS6MemoryFUlUl");
        KURIBO_EXPORT_AS(BetterSMS::Memory::hmalloc, "hmalloc__Q29BetterSMS6MemoryFP7JKRHeapUlUl");
        KURIBO_EXPORT_AS(BetterSMS::Memory::hcalloc, "hcalloc__Q29BetterSMS6MemoryFP7JKRHeapUlUl");
        KURIBO_EXPORT_AS(BetterSMS::Memory::free, "free__Q29BetterSMS6MemoryFPCv");
        KURIBO_EXPORT_AS(BetterSMS::PowerPC::getBranchDest,
                         "getBranchDest__Q29BetterSMS6MemoryFPUl");
        KURIBO_EXPORT_AS(BetterSMS::PowerPC::writeU8, "writeU8__Q29BetterSMS6MemoryFPUcUc");
        KURIBO_EXPORT_AS(BetterSMS::PowerPC::writeU16,
                         "writeU16__Q29BetterSMS6MemoryFPUsUs");
        KURIBO_EXPORT_AS(BetterSMS::PowerPC::writeU32,
                         "writeU32__Q29BetterSMS6MemoryFPUlUl");

        /* LOADING */
        KURIBO_EXPORT_AS(BetterSMS::Loading::setLoading, "setLoading__Q29BetterSMS7LoadingFb");
        KURIBO_EXPORT_AS(BetterSMS::Loading::setLoadingIconB, "setLoadingIconB__Q29BetterSMS7LoadingFPPC7ResTIMGUl");
        KURIBO_EXPORT_AS(BetterSMS::Loading::setLoadingIconW, "setLoadingIconW__Q29BetterSMS7LoadingFPPC7ResTIMGUl");
        KURIBO_EXPORT_AS(BetterSMS::Loading::setFullScreenLayout, "setFullScreenLayout__Q29BetterSMS7LoadingFP9J2DScreen");
        KURIBO_EXPORT_AS(BetterSMS::Loading::setWideScreenLayout, "setWideScreenLayout__Q29BetterSMS7LoadingFP9J2DScreen");
        KURIBO_EXPORT_AS(BetterSMS::Loading::setFrameRate, "setFrameRate__Q29BetterSMS7LoadingFf");

        /* PLAYER */
        KURIBO_EXPORT_AS(BetterSMS::Player::getRegisteredData, "getRegisteredData__Q29BetterSMS6PlayerFP6TMarioPCc");
        KURIBO_EXPORT_AS(BetterSMS::Player::getData, "getData__Q29BetterSMS6PlayerFP6TMario");
        KURIBO_EXPORT_AS(BetterSMS::Player::registerData, "registerData__Q29BetterSMS6PlayerFP6TMarioPCcPv");
        KURIBO_EXPORT_AS(BetterSMS::Player::deregisterData, "deregisterData__Q29BetterSMS6PlayerFP6TMarioPCc");
        KURIBO_EXPORT_AS(BetterSMS::Player::registerInitProcess,
                         "registerInitProcess__Q29BetterSMS6PlayerFPCcPFP6TMariob_v");
        KURIBO_EXPORT_AS(BetterSMS::Player::registerUpdateProcess,
                         "registerUpdateProcess__Q29BetterSMS6PlayerFPCcPFP6TMariob_v");
        KURIBO_EXPORT_AS(BetterSMS::Player::registerStateMachine,
                         "registerStateMachine__Q29BetterSMS6PlayerFUlPFP6TMariob_v");
        KURIBO_EXPORT_AS(BetterSMS::Player::registerCollisionHandler,
                         "registerCollisionHandler__Q29BetterSMS6PlayerFUsPFP6TMarioPC12TBGCheckDataUl_v");
        KURIBO_EXPORT_AS(BetterSMS::Player::deregisterInitProcess,
                         "deregisterInitProcess__Q29BetterSMS6PlayerFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Player::deregisterUpdateProcess,
                         "deregisterUpdateProcess__Q29BetterSMS6PlayerFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Player::deregisterStateMachine,
                         "deregisterStateMachine__Q29BetterSMS6PlayerFUl");
        KURIBO_EXPORT_AS(BetterSMS::Player::deregisterCollisionHandler,
                         "deregisterCollisionHandler__Q29BetterSMS6PlayerFUs");
        KURIBO_EXPORT_AS(BetterSMS::Player::warpToCollisionFace,
                         "warpToCollisionFace__Q29BetterSMS6PlayerFv");
        KURIBO_EXPORT_AS(BetterSMS::Player::warpToPoint, "warpToPoint__Q29BetterSMS6PlayerFv");
        KURIBO_EXPORT_AS(BetterSMS::Player::rotateRelativeToCamera,
                         "rotateRelativeToCamera__Q29BetterSMS6PlayerFv");
        KURIBO_EXPORT_AS(BetterSMS::Player::setFire, "setFire__Q29BetterSMS6PlayerFv");
        KURIBO_EXPORT_AS(BetterSMS::Player::extinguishFire, "extinguishFire__Q29BetterSMS6PlayerFv");

        /* MUSIC */
        KURIBO_EXPORT_AS(BetterSMS::Music::queueSong, "queueSong__Q29BetterSMS5MusicFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Music::playSong, "playSong__Q29BetterSMS5MusicFv");
        KURIBO_EXPORT_AS(BetterSMS::Music::pauseSong, "pauseSong__Q29BetterSMS5MusicFf");
        KURIBO_EXPORT_AS(BetterSMS::Music::stopSong, "stopSong__Q29BetterSMS5MusicFf");
        KURIBO_EXPORT_AS(BetterSMS::Music::skipSong, "skipSong__Q29BetterSMS5MusicFf");
        KURIBO_EXPORT_AS(BetterSMS::Music::setVolume, "setVolume__Q29BetterSMS5MusicFUcUc");
        KURIBO_EXPORT_AS(BetterSMS::Music::setVolumeFade, "setVolumeFade__Q29BetterSMS5MusicFUcf");
        KURIBO_EXPORT_AS(BetterSMS::Music::setLoopPoint, "setLoopPoint__Q29BetterSMS5MusicFff");
        KURIBO_EXPORT_AS(BetterSMS::Music::getAudioStreamer, "getAudioStreamer__Q29BetterSMS5MusicFv");

        /* SETTINGS */
        KURIBO_EXPORT_AS(BetterSMS::Settings::getGroup, "getGroup__Q29BetterSMS8SettingsFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Settings::isGroupRegistered,
                         "isGroupRegistered__Q29BetterSMS8SettingsFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Settings::registerGroup, "registerGroup__Q29BetterSMS8SettingsFPCcPQ39BetterSMS8Settings13SettingsGroup");
        KURIBO_EXPORT_AS(BetterSMS::Settings::deregisterGroup,
                         "deregisterGroup__Q29BetterSMS8SettingsFPCc");

    /* OBJECTS */
    #if BETTER_SMS_EXTRA_OBJECTS
        KURIBO_EXPORT_AS(BetterSMS::Objects::isObjectRegistered,
                         "isObjectRegistered__Q29BetterSMS7ObjectsFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Objects::registerObjectAsMapObj,
                         "registerObjectAsMapObj__Q29BetterSMS7ObjectsFPCcP7ObjDataP7ObjDataPFv_PQ26JDrama8TNameRef");
        KURIBO_EXPORT_AS(BetterSMS::Objects::registerObjectAsEnemy,
                         "registerObjectAsEnemy__Q29BetterSMS7ObjectsFPCcP7ObjDataP7ObjDataPFv_PQ26JDrama8TNameRef");
        KURIBO_EXPORT_AS(BetterSMS::Objects::registerObjectAsMisc,
                         "registerObjectAsMisc__Q29BetterSMS7ObjectsFPCcP7ObjDataP7ObjDataPFv_PQ26JDrama8TNameRef");
        KURIBO_EXPORT_AS(BetterSMS::Objects::deregisterObject,
                         "deregisterObject__Q29BetterSMS7ObjectsFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Objects::getRegisteredObjectCount,
                         "getRegisteredObjectCount__Q29BetterSMS7ObjectsFv");
        KURIBO_EXPORT_AS(BetterSMS::Objects::getRegisteredCustomObjectCount,
                         "getRegisteredCustomObjectCount__Q29BetterSMS7ObjectsFv");
        KURIBO_EXPORT_AS(BetterSMS::Objects::getRemainingCapacity,
                         "getRemainingCapacity__Q29BetterSMS7ObjectsFv");
    #endif

        /* GAME */
        KURIBO_EXPORT_AS(BetterSMS::Game::isOnInitRegistered,
                         "isOnInitRegistered__Q29BetterSMS4GameFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Game::isOnBootRegistered,
                         "isOnBootRegistered__Q29BetterSMS4GameFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Game::isOnLoopRegistered,
                         "isOnLoopRegistered__Q29BetterSMS4GameFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Game::isOnPostDrawRegistered,
                         "isOnPostDrawRegistered__Q29BetterSMS4GameFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Game::isOnChangeRegistered,
                         "isOnChangeRegistered__Q29BetterSMS4GameFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Game::registerOnInitCallback,
                         "registerOnInitCallback__Q29BetterSMS4GameFPCcPFP12TApplication_v");
        KURIBO_EXPORT_AS(BetterSMS::Game::registerOnBootCallback,
                         "registerOnBootCallback__Q29BetterSMS4GameFPCcPFP12TApplication_v");
        KURIBO_EXPORT_AS(
            BetterSMS::Game::registerOnLoopCallback,
            "registerOnLoopCallback__Q29BetterSMS4GameFPCcPFP12TApplication_v");
        KURIBO_EXPORT_AS(BetterSMS::Game::registerOnPostDrawCallback,
                         "registerOnPostDrawCallback__Q29BetterSMS4GameFPCcPFP12TApplicationPC13J2DOrthoGraph_v");
        KURIBO_EXPORT_AS(BetterSMS::Game::registerOnChangeCallback,
                         "registerOnChangeCallback__Q29BetterSMS4GameFPCcPFP12TApplication_v");
        KURIBO_EXPORT_AS(BetterSMS::Game::deregisterOnInitCallback,
                         "deregisterOnInitCallback__Q29BetterSMS4GameFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Game::deregisterOnBootCallback,
                         "deregisterOnBootCallback__Q29BetterSMS4GameFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Game::deregisterOnLoopCallback,
                         "deregisterOnLoopCallback__Q29BetterSMS4GameFPCc");
        KURIBO_EXPORT_AS(
            BetterSMS::Game::deregisterOnPostDrawCallback,
            "deregisterOnPostDrawCallback__Q29BetterSMS4GameFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Game::deregisterOnChangeCallback,
                         "deregisterOnChangeCallback__Q29BetterSMS4GameFPCc");

    //    /* STAGE */
        KURIBO_EXPORT_AS(BetterSMS::Stage::getStageConfiguration,
                         "getStageConfiguration__Q29BetterSMS5StageFv");
        KURIBO_EXPORT_AS(BetterSMS::Stage::getStageName, "getStageName__Q29BetterSMS5StageFP12TApplication");
        KURIBO_EXPORT_AS(BetterSMS::Stage::isStageInitRegistered,
                         "isStageInitRegistered__Q29BetterSMS5StageFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Stage::isStageUpdateRegistered,
                         "isStageUpdateRegistered__Q29BetterSMS5StageFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Stage::isDraw2DRegistered,
                         "isDraw2DRegistered__Q29BetterSMS5StageFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Stage::registerInitCallback,
                         "registerInitCallback__Q29BetterSMS5StageFPCcPFP12TMarDirector_v");
        KURIBO_EXPORT_AS(BetterSMS::Stage::registerUpdateCallback,
                         "registerUpdateCallback__Q29BetterSMS5StageFPCcPFP12TMarDirector_v");
        KURIBO_EXPORT_AS(BetterSMS::Stage::registerDraw2DCallback,
                         "registerDraw2DCallback__Q29BetterSMS5StageFPCcPFP12TMarDirectorPC13J2DOrthoGraph_v");
        KURIBO_EXPORT_AS(BetterSMS::Stage::deregisterInitCallback,
                         "deregisterInitCallback__Q29BetterSMS5StageFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Stage::deregisterUpdateCallback,
                         "deregisterUpdateCallback__Q29BetterSMS5StageFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Stage::deregisterDraw2DCallback,
                         "deregisterDraw2DCallback__Q29BetterSMS5StageFPCc");

        /* TIME */
        KURIBO_EXPORT_AS(BetterSMS::Time::buildDate, "buildDate__Q29BetterSMS4TimeFv");
        KURIBO_EXPORT_AS(BetterSMS::Time::buildTime, "buildTime__Q29BetterSMS4TimeFv");
        KURIBO_EXPORT_AS(BetterSMS::Time::calendarTime, "calendarTime__Q29BetterSMS4TimeFR14OSCalendarTime");
        KURIBO_EXPORT_AS(BetterSMS::Time::calendarToDate, "calendarToDate__Q29BetterSMS4TimeFPcRC14OSCalendarTime");
        KURIBO_EXPORT_AS(BetterSMS::Time::calendarToTime, "calendarToTime__Q29BetterSMS4TimeFPcRC14OSCalendarTime");
        KURIBO_EXPORT_AS(BetterSMS::Time::date, "date__Q29BetterSMS4TimeFPc");
        KURIBO_EXPORT_AS(BetterSMS::Time::day, "day__Q29BetterSMS4TimeFv");
        KURIBO_EXPORT_AS(BetterSMS::Time::hour, "hour__Q29BetterSMS4TimeFv");
        KURIBO_EXPORT_AS(BetterSMS::Time::microsecond, "microsecond__Q29BetterSMS4TimeFv");
        KURIBO_EXPORT_AS(BetterSMS::Time::millisecond, "millisecond__Q29BetterSMS4TimeFv");
        KURIBO_EXPORT_AS(BetterSMS::Time::minute, "minute__Q29BetterSMS4TimeFv");
        KURIBO_EXPORT_AS(BetterSMS::Time::month, "month__Q29BetterSMS4TimeFv");
        KURIBO_EXPORT_AS(BetterSMS::Time::nanosecond, "nanosecond__Q29BetterSMS4TimeFv");
        KURIBO_EXPORT_AS(BetterSMS::Time::ostime, "ostime__Q29BetterSMS4TimeFv");
        KURIBO_EXPORT_AS(BetterSMS::Time::second, "second__Q29BetterSMS4TimeFv");
        KURIBO_EXPORT_AS(BetterSMS::Time::time, "time__Q29BetterSMS4TimeFPc");
        KURIBO_EXPORT_AS(BetterSMS::Time::year, "year__Q29BetterSMS4TimeFv");

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

        /* STRLIB */
        KURIBO_EXPORT(reverse);
        KURIBO_EXPORT(itoa);
    }
    KURIBO_EXECUTE_ON_UNLOAD { destroyLib(); }

}
KURIBO_MODULE_END()

extern "C" void __cxa_pure_virtual() { SMS_ASM_BLOCK("trap \r\n"); }
