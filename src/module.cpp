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

// Kuribo/Kamek
#include "common_sdk.h"

// SETTINGS //
static Settings::SettingsGroup sSettingsGroup("Better Sunshine Engine");

// Sunshine settings
RumbleSetting gRumbleSetting("Controller Rumble");
SoundSetting gSoundSetting("Sound Mode");
SubtitleSetting gSubtitleSetting("Movie Subtitles");
//

// BetterSMS settings
static bool sBugFixesFlag = true;
Settings::SwitchSetting gBugFixesSetting("Bug & Exploit Fixes", &sBugFixesFlag);

AspectRatioSetting gAspectRatioSetting("Aspect Ratio");
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
extern void updateDebugCallbacks();
extern void drawDebugCallbacks();

extern void initMarioXYZMode(TApplication *);
extern void updateMarioXYZMode(TApplication *);

extern void drawMonitor(TApplication *, J2DOrthoGraph *);
extern void resetMonitor(TApplication *);

extern void initFPSMonitor(TApplication *);
extern void updateFPSMonitor(TApplication *);
extern void drawFPSMonitor(TApplication *, J2DOrthoGraph *);

extern void initStateMonitor(TApplication *);
extern void updateStateMonitor(TApplication *);
extern void drawStateMonitor(TApplication *, J2DOrthoGraph *);

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
extern void checkForYoshiWaterDeath(TMario *player, bool isMario);
extern void forceValidRidingAnimation(TMario *player, bool isMario);

// MUSIC
extern void initStreamInfo(TApplication *app);
extern void printStreamInfo(TApplication *app, J2DOrthoGraph *graph);

// LOADING SCREEN
extern void initLoadingScreen();

static TMarDirector *initLib() {

    TMarDirector *director;
    SMS_FROM_GPR(26, director);

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
    sSettingsGroup.addSetting(&gRumbleSetting);
    sSettingsGroup.addSetting(&gSoundSetting);
    sSettingsGroup.addSetting(&gSubtitleSetting);

    sSettingsGroup.addSetting(&gBugFixesSetting);
    sSettingsGroup.addSetting(&gAspectRatioSetting);

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
    Player::registerUpdateProcess("__update_yoshi_swim", checkForYoshiWaterDeath);
    Player::registerUpdateProcess("__update_yoshi_riding", forceValidRidingAnimation);

    // DEBUG
    Debug::registerInitCallback("__init_debug_xyz", initMarioXYZMode);
    Debug::registerUpdateCallback("__update_debug_xyz", updateMarioXYZMode);

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

    return director;
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

#if defined(SMS_BUILD_KURIBO) && !defined(SMS_BUILD_KAMEK) && !defined(SMS_BUILD_KAMEK_INLINE)

KURIBO_MODULE_BEGIN(BETTER_SMS_MODULE_NAME, BETTER_SMS_AUTHOR_NAME, BETTER_SMS_VERSION_TAG) {
    KURIBO_EXECUTE_ON_LOAD { initLib(); }
    KURIBO_EXECUTE_ON_UNLOAD { destroyLib(); }

    // Generate exports
    /* BMG */
    KURIBO_EXPORT(BetterSMS::BMG::isBMGCommandRegistered);
    KURIBO_EXPORT(BetterSMS::BMG::registerBMGCommandCallback);
    KURIBO_EXPORT(BetterSMS::BMG::deregisterBMGCommandCallback);

    /* DEBUG */
    KURIBO_EXPORT(BetterSMS::Debug::isInitRegistered);
    KURIBO_EXPORT(BetterSMS::Debug::isUpdateRegistered);
    KURIBO_EXPORT(BetterSMS::Debug::registerInitCallback);
    KURIBO_EXPORT(BetterSMS::Debug::registerUpdateCallback);
    KURIBO_EXPORT(BetterSMS::Debug::deregisterInitCallback);
    KURIBO_EXPORT(BetterSMS::Debug::deregisterUpdateCallback);

    /* LOGGING */
    KURIBO_EXPORT(BetterSMS::Console::log);
    KURIBO_EXPORT(BetterSMS::Console::emulatorLog);
    KURIBO_EXPORT(BetterSMS::Console::hardwareLog);
    KURIBO_EXPORT(BetterSMS::Console::debugLog);

    /* MEMORY */
    KURIBO_EXPORT(BetterSMS::Memory::malloc);
    KURIBO_EXPORT(BetterSMS::Memory::calloc);
    KURIBO_EXPORT(BetterSMS::Memory::hmalloc);
    KURIBO_EXPORT(BetterSMS::Memory::hcalloc);
    KURIBO_EXPORT(BetterSMS::Memory::free);
    KURIBO_EXPORT(BetterSMS::PPC::getBranchDest);
    KURIBO_EXPORT(BetterSMS::PPC::writeU8);
    KURIBO_EXPORT(BetterSMS::PPC::writeU16);
    KURIBO_EXPORT(BetterSMS::PPC::writeU32);

    /* MUSIC */
    KURIBO_EXPORT(BetterSMS::Music::queueSong);
    KURIBO_EXPORT(BetterSMS::Music::playSong);
    KURIBO_EXPORT(BetterSMS::Music::pauseSong);
    KURIBO_EXPORT(BetterSMS::Music::stopSong);
    KURIBO_EXPORT(BetterSMS::Music::skipSong);
    KURIBO_EXPORT(BetterSMS::Music::setVolume);
    KURIBO_EXPORT(BetterSMS::Music::setVolumeFade);
    KURIBO_EXPORT(BetterSMS::Music::setLoopPoint);
    KURIBO_EXPORT(BetterSMS::Music::getAudioStreamer);

/* OBJECTS */
#if BETTER_SMS_EXTRA_OBJECTS
    KURIBO_EXPORT(BetterSMS::Objects::isObjectRegistered);
    KURIBO_EXPORT(BetterSMS::Objects::registerObjectAsMapObj);
    KURIBO_EXPORT(BetterSMS::Objects::registerObjectAsEnemy);
    KURIBO_EXPORT(BetterSMS::Objects::registerObjectAsMisc);
    KURIBO_EXPORT(BetterSMS::Objects::deregisterObject);
    KURIBO_EXPORT(BetterSMS::Objects::getRegisteredObjectCount);
    KURIBO_EXPORT(BetterSMS::Objects::getRegisteredCustomObjectCount);
    KURIBO_EXPORT(BetterSMS::Objects::getRemainingCapacity);
#endif

    /* STAGE */
    KURIBO_EXPORT(BetterSMS::Stage::getStageConfiguration);
    KURIBO_EXPORT(BetterSMS::Stage::getStageName);
    KURIBO_EXPORT(BetterSMS::Stage::isStageInitRegistered);
    KURIBO_EXPORT(BetterSMS::Stage::isStageUpdateRegistered);
    KURIBO_EXPORT(BetterSMS::Stage::isDraw2DRegistered);
    KURIBO_EXPORT(BetterSMS::Stage::registerInitCallback);
    KURIBO_EXPORT(BetterSMS::Stage::registerUpdateCallback);
    KURIBO_EXPORT(BetterSMS::Stage::registerDraw2DCallback);
    KURIBO_EXPORT(BetterSMS::Stage::deregisterInitCallback);
    KURIBO_EXPORT(BetterSMS::Stage::deregisterUpdateCallback);
    KURIBO_EXPORT(BetterSMS::Stage::deregisterDraw2DCallback);

    /* TIME */
    KURIBO_EXPORT(BetterSMS::Time::buildDate);
    KURIBO_EXPORT(BetterSMS::Time::buildTime);
    KURIBO_EXPORT(BetterSMS::Time::calendarTime);
    KURIBO_EXPORT(BetterSMS::Time::calendarToDate);
    KURIBO_EXPORT(BetterSMS::Time::calendarToTime);
    KURIBO_EXPORT(BetterSMS::Time::date);
    KURIBO_EXPORT(BetterSMS::Time::day);
    KURIBO_EXPORT(BetterSMS::Time::hour);
    KURIBO_EXPORT(BetterSMS::Time::microsecond);
    KURIBO_EXPORT(BetterSMS::Time::millisecond);
    KURIBO_EXPORT(BetterSMS::Time::minute);
    KURIBO_EXPORT(BetterSMS::Time::month);
    KURIBO_EXPORT(BetterSMS::Time::nanosecond);
    KURIBO_EXPORT(BetterSMS::Time::ostime);
    KURIBO_EXPORT(BetterSMS::Time::second);
    KURIBO_EXPORT(BetterSMS::Time::time);
    KURIBO_EXPORT(BetterSMS::Time::year);

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
KURIBO_MODULE_END()

#else

#error "BetterSMS only supports Kuribo style modulation"

#endif
