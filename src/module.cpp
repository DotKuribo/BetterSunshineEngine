#include <Dolphin/OS.h>
#include <SMS/macros.h>

#include "collision/warp.hxx"

// BetterSMS API
#include "bmg.hxx"
#include "cstd/ctype.h"
#include "cstd/stdlib.h"
#include "debug.hxx"
#include "logging.hxx"
#include "memory.hxx"
#include "module.hxx"
#include "music.hxx"
#include "object.hxx"
#include "player.hxx"
#include "stage.hxx"
#include "time.hxx"

// Kuribo/Kamek
#include "common_sdk.h"

static bool sIsAudioStreaming     = false;
static bool sIsAudioStreamAllowed = true;
#if SMS_DEBUG
static bool sIsDebugMode = true;
#else
static bool sIsDebugMode = false;
#endif

SMS_NO_INLINE bool BetterSMS::isGameEmulated() {
    return BootInfo.mConsoleType == OS_CONSOLE_DEV_KIT3;
}
SMS_NO_INLINE bool BetterSMS::isMusicBeingStreamed() { return sIsAudioStreaming; }

SMS_NO_INLINE bool BetterSMS::isDebugMode() { return sIsDebugMode; }
SMS_NO_INLINE void BetterSMS::setDebugMode(bool active) { sIsDebugMode = active; }

SMS_NO_INLINE bool BetterSMS::isMusicStreamingAllowed() { return sIsAudioStreamAllowed; }
SMS_NO_INLINE void BetterSMS::setMusicStreamingAllowed(bool allowed) {
    sIsAudioStreamAllowed = allowed;
}

// mario.cpp
extern "C" s16 mario_shadowCrashPatch();

// shine.cpp
extern "C" void shine_animationFreezeCheck();
extern "C" void shine_thinkCloseCamera();

// ================================= //

using namespace BetterSMS;

extern void makeExtendedObjDataTable();

// DEBUG
extern void initDebugCallbacks(TMarDirector *);
extern void updateDebugCallbacks(TMarDirector *);
extern void drawDebugCallbacks(TMarDirector *, J2DOrthoGraph *);

// STAGE CONFIG
extern void resetGlobalValues(TMarDirector *);
extern void loadStageConfig(TMarDirector *);

// PLAYER CONFIG
extern void initMario(TMario *, bool);

// PLAYER WARP
extern void processWarp(TMario *, bool);

static void initLib() {
    makeExtendedObjDataTable();

    // Set up debug handlers
    Stage::registerInitCallback("__init_debug", initDebugCallbacks);
    Stage::registerUpdateCallback("__update_debug", updateDebugCallbacks);
    Stage::registerDraw2DCallback("__draw_debug", drawDebugCallbacks);

    // Set up stage config handlers
    Stage::registerInitCallback("__init_globals", resetGlobalValues);
    Stage::registerInitCallback("__init_config", loadStageConfig);

    // Set up player params
    Player::registerInitProcess("__init_mario", initMario);

    // Set up player warp handler
    Player::registerUpdateProcess("__update_mario_warp", processWarp);

    // Set up loading screen
    Stage::registerInitCallback("__init_load_screen", (Stage::StageInitCallback)0);
    Stage::registerUpdateCallback("__update_load_screen", (Stage::StageUpdateCallback)0);
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

KURIBO_MODULE_BEGIN(BETTER_SMS_MODULE_NAME, BETTER_SMS_AUTHOR_NAME, BETTER_SMS_VERSION_TAG);
{
    KURIBO_EXECUTE_ON_LOAD(initLib);
    KURIBO_EXECUTE_ON_UNLOAD(destroyLib);

    // Generate exports
    /* BMG */
    KURIBO_EXPORT(BetterSMS::BMG::isBMGCommandRegistered);
    KURIBO_EXPORT(BetterSMS::BMG::registerBMGCommandCallback);
    KURIBO_EXPORT(BetterSMS::BMG::deregisterBMGCommandCallback);

    /* DEBUG */
    KURIBO_EXPORT(BetterSMS::Debug::isDebugInitRegistered);
    KURIBO_EXPORT(BetterSMS::Debug::isDebugUpdateRegistered);
    KURIBO_EXPORT(BetterSMS::Debug::registerDebugInitCallback);
    KURIBO_EXPORT(BetterSMS::Debug::registerDebugUpdateCallback);
    KURIBO_EXPORT(BetterSMS::Debug::deregisterDebugInitCallback);
    KURIBO_EXPORT(BetterSMS::Debug::deregisterDebugUpdateCallback);

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
KURIBO_MODULE_END();

#else

#error "BetterSMS only supports Kuribo style modulation"

#endif
