#include <Dolphin/OS.h>
#include <Dolphin/math.h>

#include <SMS/macros.h>

#include "libs/global_unordered_map.hxx"
#include "libs/string.hxx"

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
#include "objects/fog.hxx"
#include "objects/generic.hxx"
#include "player.hxx"
#include "p_module.hxx"
#include "p_settings.hxx"
#include "settings.hxx"
#include "stage.hxx"
#include "time.hxx"

// SETTINGS //

// BetterSMS settings
static Settings::SettingsGroup sSettingsGroup(2, 0, Settings::Priority::CORE);

BugsSetting gBugFixesSetting("Bug Fixes");
BugsSetting gExploitFixesSetting("Exploit Fixes");
BugsSetting gCollisionFixesSetting("Collision Fixes");
AspectRatioSetting gAspectRatioSetting("Aspect Ratio");
ViewportSetting gViewportSetting("Viewport");
FPSSetting gFPSSetting("Frame Rate");
static bool sCameraInvertX = false;
static bool sCameraInvertY = false;
Settings::SwitchSetting gCameraInvertXSetting("Invert Camera X", &sCameraInvertX);
Settings::SwitchSetting gCameraInvertYSetting("Invert Camera Y", &sCameraInvertY);
PromptsSetting gSavePromptSetting("Save Prompts");

static BetterSMS::ModuleInfo sBetterSMSInfo{"Better Sunshine Engine", 1, 1, &sSettingsGroup};
//

BetterSMS::TGlobalUnorderedMap<TGlobalString, const BetterSMS::ModuleInfo *> gModuleInfos(16);

const ModuleInfo *BetterSMS::getModuleInfo(const char *key) { return gModuleInfos.at(key); }

bool BetterSMS::isModuleRegistered(const char *key) { return gModuleInfos.find(key) != gModuleInfos.end(); }
bool BetterSMS::registerModule(const ModuleInfo *info) {
    if (isModuleRegistered(info->mName)) {
        OSPanic(__FILE__, __LINE__,
                "Module \"%s\" is trying to register under the name \"%s\", which is "
                "already taken!",
                info->mName, info->mName);
        return false;
    }
    gModuleInfos[info->mName] = info;
    return true;
}
void BetterSMS::deregisterModule(const ModuleInfo *info) {
    gModuleInfos.erase(info->mName);
}

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

extern u32 XYZState;
extern void checkMarioXYZMode(TMario *, bool);
extern bool updateMarioXYZMode(TMario *);
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
extern u32 MultiJumpState;
extern void checkForMultiJump(TMario *, bool);
extern bool processMultiJump(TMario *);
extern void updateDeadTriggerState(TMario *player, bool isMario);
extern void blazePlayer(TMario *player, bool isMario);

// PLAYER STATE
extern void updateClimbContext(TMario *, bool);

// PLAYER COLLISION
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

// SETTINGS
extern void initAllSettings(TApplication *);

extern void initUnlockedSettings(TApplication *);
extern void checkForCompletionAwards(TApplication *);
extern void updateUnlockedSettings(TApplication *);
extern void drawUnlockedSettings(TApplication *, const J2DOrthoGraph *);

extern "C" void __cxa_pure_virtual();

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
    sSettingsGroup.addSetting(&gBugFixesSetting);
    sSettingsGroup.addSetting(&gExploitFixesSetting);
    sSettingsGroup.addSetting(&gCollisionFixesSetting);
    sSettingsGroup.addSetting(&gViewportSetting);
    sSettingsGroup.addSetting(&gAspectRatioSetting);
    sSettingsGroup.addSetting(&gFPSSetting);
    sSettingsGroup.addSetting(&gCameraInvertXSetting);
    sSettingsGroup.addSetting(&gCameraInvertYSetting);
    sSettingsGroup.addSetting(&gSavePromptSetting);
    {
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
        saveInfo.mSaveGlobal  = true;
    }
    BetterSMS::registerModule(&sBetterSMSInfo);

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

    //// Set up stage config handlers
    Stage::registerInitCallback("__init_config", loadStageConfig);
    Stage::registerExitCallback("__reset_globals", resetGlobalValues);

    //// Set up player params
    Player::registerInitCallback("__init_mario", initMario);
    Stage::registerExitCallback("__destroy_mario", resetPlayerDatas);

    Player::registerUpdateCallback("__update_mario_multijump", checkForMultiJump);
    Player::registerStateMachine(MultiJumpState, processMultiJump);

    #if 0
    Player::registerUpdateCallback("__debug_warp_collision", updateDebugCollision);
    #endif
    Player::registerUpdateCallback("__blaze_player", blazePlayer);

    // Set up player map collisions
    Player::registerCollisionHandler(3000, decHealth);
    Player::registerCollisionHandler(3001, incHealth);
    Player::registerCollisionHandler(3010, elecPlayer);
    Player::registerCollisionHandler(3011, burnPlayer);
    Player::registerCollisionHandler(3030, changeNozzleSprayOnTouch);
    Player::registerCollisionHandler(3031, changeNozzleHoverOnTouch);
    Player::registerCollisionHandler(3032, changeNozzleTurboOnTouch);
    Player::registerCollisionHandler(3033, changeNozzleRocketOnTouch);
    Player::registerCollisionHandler(3035, changeNozzleSpray);
    Player::registerCollisionHandler(3036, changeNozzleHover);
    Player::registerCollisionHandler(3037, changeNozzleTurbo);
    Player::registerCollisionHandler(3038, changeNozzleRocket);
    Player::registerCollisionHandler(3040, setGravityCol);
    Player::registerCollisionHandler(3041, antiGravityCol);
    Player::registerCollisionHandler(3042, boostPadCol);
    Player::registerCollisionHandler(3060, instantWarpHandler);
    Player::registerCollisionHandler(3061, screenWipeWarpHandler);
    Player::registerCollisionHandler(3062, effectWarpHandler);
    Player::registerCollisionHandler(3063, instantScreenWipeWarpHandler);
    Player::registerCollisionHandler(3064, portalWarpHandler);
    Player::registerCollisionHandler(3065, portalFreeWarpHandler);

    //// PLAYER STATE
    Player::registerUpdateCallback("__check_upwarp_climb", updateClimbContext);

    //// YOSHI
    Player::registerUpdateCallback("__update_yoshi_swim", checkForYoshiDeath);
    Player::registerUpdateCallback("__update_yoshi_riding", forceValidRidingAnimation);

    //// DEBUG
    Player::registerUpdateCallback("__check_for_xyz", checkMarioXYZMode);
    Player::registerUpdateCallback("__update_dead_trigger", updateDeadTriggerState);
    Player::registerStateMachine(XYZState, updateMarioXYZMode);
    Debug::registerUpdateCallback("__update_fludd_nozzle", updateFluddNozzle);

    Debug::registerDrawCallback("__draw_memory_usage", drawMonitor);
    Stage::registerExitCallback("__reset_memory_usage", resetMonitor);

    Debug::registerInitCallback("__init_fps_counter", initFPSMonitor);
    Debug::registerUpdateCallback("__update_fps_counter", updateFPSMonitor);
    Debug::registerDrawCallback("__draw_fps_counter", drawFPSMonitor);

    Debug::registerInitCallback("__init_state_counter", initStateMonitor);
    Debug::registerUpdateCallback("__update_state_counter", updateStateMonitor);
    Debug::registerDrawCallback("__draw_state_counter", drawStateMonitor);

    // Music
    Game::registerChangeCallback("__stop_music_on_exit", stopMusicOnExitStage);
    /*Debug::registerInitCallback("__init_music_state", initStreamInfo);
    Debug::registerDrawCallback("__draw_music_state", printStreamInfo);*/

    Objects::registerObjectAsMapObj("GenericRailObj", &generic_railobj_data,
                                    TGenericRailObj::instantiate);
    Objects::registerObjectAsMisc("SimpleFog", TSimpleFog::instantiate);

    Game::registerBootCallback("__init_debug_handles", initDebugCallbacks);

    ////Game::registerOnBootCallback("__init_fps", updateFPSBoot);
    Stage::registerInitCallback("__init_fps", updateFPS);
    Stage::registerUpdateCallback("__update_fps", updateFPS);

    //// SETTINGS
    Game::registerInitCallback("__load_settings", initAllSettings);
    Game::registerBootCallback("__init_setting_notifs", initUnlockedSettings);
    Game::registerLoopCallback("__update_setting_notifs", updateUnlockedSettings);
    Game::registerPostDrawCallback("__draw_setting_notifs", drawUnlockedSettings);
    Debug::registerUpdateCallback("__check_awards", checkForCompletionAwards);

    PowerPC::writeU32(reinterpret_cast<u32 *>(0x802A7454), 0x3C600004); // Make system memory more expansive (takes away from stage heap)
}

static void destroyLib() {
    PowerPC::writeU32(reinterpret_cast<u32 *>(0x802A7454), 0x3C600002);
}

KURIBO_MODULE_BEGIN(BETTER_SMS_MODULE_NAME, BETTER_SMS_AUTHOR_NAME, BETTER_SMS_VERSION_TAG)
{
    KURIBO_EXECUTE_ON_LOAD {
        initLib();

        // Generate exports

        /* MODULE */
        // Hack for circular reference
        KURIBO_EXPORT_AS(BetterSMS::Settings::getGroupName,
                         "getGroupName__Q29BetterSMS8SettingsFRCQ39BetterSMS8Settings13SettingsGroup");
        KURIBO_EXPORT_AS(BetterSMS::getModuleInfo, "getModuleInfo__9BetterSMSFPCc");
        KURIBO_EXPORT_AS(BetterSMS::isModuleRegistered, "isModuleRegistered__9BetterSMSFPCc");
        KURIBO_EXPORT_AS(BetterSMS::registerModule, "registerModule__9BetterSMSFPCQ29BetterSMS10ModuleInfo");
        KURIBO_EXPORT_AS(BetterSMS::deregisterModule, "deregisterModule__9BetterSMSFPCQ29BetterSMS10ModuleInfo");
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

        /* APPLICATION */
        KURIBO_EXPORT_AS(BetterSMS::Application::isContextRegistered, "isContextRegistered__Q29BetterSMS11ApplicationFUc");
        KURIBO_EXPORT_AS(BetterSMS::Application::registerContextCallback, "registerContextCallback__Q29BetterSMS11ApplicationFUcPFP12TApplication_b");
        KURIBO_EXPORT_AS(BetterSMS::Application::deregisterContextCallback, "deregisterContextCallback__Q29BetterSMS11ApplicationFUc");

        /* BMG */
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
                         "getBranchDest__Q29BetterSMS7PowerPCFPUl");
        KURIBO_EXPORT_AS(BetterSMS::PowerPC::writeU8, "writeU8__Q29BetterSMS7PowerPCFPUcUc");
        KURIBO_EXPORT_AS(BetterSMS::PowerPC::writeU16,
                         "writeU16__Q29BetterSMS7PowerPCFPUsUs");
        KURIBO_EXPORT_AS(BetterSMS::PowerPC::writeU32,
                         "writeU32__Q29BetterSMS7PowerPCFPUlUl");

        /* LOADING */
        KURIBO_EXPORT_AS(BetterSMS::Loading::setLoading, "setLoading__Q29BetterSMS7LoadingFb");
        KURIBO_EXPORT_AS(BetterSMS::Loading::setLoadingIcon, "setLoadingIcon__Q29BetterSMS7LoadingFPPC7ResTIMGUl");
        KURIBO_EXPORT_AS(BetterSMS::Loading::setFullScreenLayout, "setFullScreenLayout__Q29BetterSMS7LoadingFP9J2DScreen");
        KURIBO_EXPORT_AS(BetterSMS::Loading::setWideScreenLayout,
                         "setWideScreenLayout__Q29BetterSMS7LoadingFP9J2DScreen");
        KURIBO_EXPORT_AS(BetterSMS::Loading::setUltraWideScreenLayout,
                         "setUltraWideScreenLayout__Q29BetterSMS7LoadingFP9J2DScreen");
        KURIBO_EXPORT_AS(BetterSMS::Loading::setFrameRate, "setFrameRate__Q29BetterSMS7LoadingFf");

        /* PLAYER */
        KURIBO_EXPORT_AS(BetterSMS::Player::getRegisteredData, "getRegisteredData__Q29BetterSMS6PlayerFP6TMarioPCc");
        KURIBO_EXPORT_AS(BetterSMS::Player::getData, "getData__Q29BetterSMS6PlayerFP6TMario");
        KURIBO_EXPORT_AS(BetterSMS::Player::registerData, "registerData__Q29BetterSMS6PlayerFP6TMarioPCcPv");
        KURIBO_EXPORT_AS(BetterSMS::Player::deregisterData, "deregisterData__Q29BetterSMS6PlayerFP6TMarioPCc");
        KURIBO_EXPORT_AS(BetterSMS::Player::registerInitCallback,
                         "registerInitCallback__Q29BetterSMS6PlayerFPCcPFP6TMariob_v");
        KURIBO_EXPORT_AS(BetterSMS::Player::registerLoadAfterCallback,
                         "registerLoadAfterCallback__Q29BetterSMS6PlayerFPCcPFP6TMario_v");
        KURIBO_EXPORT_AS(BetterSMS::Player::registerUpdateCallback,
                         "registerUpdateCallback__Q29BetterSMS6PlayerFPCcPFP6TMariob_v");
        KURIBO_EXPORT_AS(BetterSMS::Player::registerStateMachine,
                         "registerStateMachine__Q29BetterSMS6PlayerFUlPFP6TMario_b");
        KURIBO_EXPORT_AS(BetterSMS::Player::registerCollisionHandler,
                         "registerCollisionHandler__Q29BetterSMS6PlayerFUsPFP6TMarioPC12TBGCheckDataUl_v");
        KURIBO_EXPORT_AS(BetterSMS::Player::deregisterInitCallback,
                         "deregisterInitCallback__Q29BetterSMS6PlayerFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Player::deregisterLoadAfterCallback,
                         "deregisterLoadAfterCallback__Q29BetterSMS6PlayerFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Player::deregisterUpdateCallback,
                         "deregisterUpdateCallback__Q29BetterSMS6PlayerFPCc");
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
        KURIBO_EXPORT_AS(BetterSMS::Music::setMaxVolume, "setMaxVolume__Q29BetterSMS5MusicFUc");
        KURIBO_EXPORT_AS(BetterSMS::Music::setLoopPoint, "setLoopPoint__Q29BetterSMS5MusicFff");

    /* OBJECTS */
    #if BETTER_SMS_EXTRA_OBJECTS
        KURIBO_EXPORT_AS(BetterSMS::Objects::registerObjectAsMapObj,
                         "registerObjectAsMapObj__Q29BetterSMS7ObjectsFPCcP7ObjDataPFv_PQ26JDrama8TNameRef");
        KURIBO_EXPORT_AS(BetterSMS::Objects::registerObjectAsEnemy,
                         "registerObjectAsEnemy__Q29BetterSMS7ObjectsFPCcP7ObjDataPFv_PQ26JDrama8TNameRef");
        KURIBO_EXPORT_AS(BetterSMS::Objects::registerObjectAsMisc,
                         "registerObjectAsMisc__Q29BetterSMS7ObjectsFPCcPFv_PQ26JDrama8TNameRef");
        KURIBO_EXPORT_AS(BetterSMS::Objects::deregisterObject,
                         "deregisterObject__Q29BetterSMS7ObjectsFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Objects::getRemainingCapacity,
                         "getRemainingCapacity__Q29BetterSMS7ObjectsFv");
    #endif

        /* GAME */
        KURIBO_EXPORT_AS(BetterSMS::Game::registerInitCallback,
                         "registerInitCallback__Q29BetterSMS4GameFPCcPFP12TApplication_v");
        KURIBO_EXPORT_AS(BetterSMS::Game::registerBootCallback,
                         "registerBootCallback__Q29BetterSMS4GameFPCcPFP12TApplication_v");
        KURIBO_EXPORT_AS(
            BetterSMS::Game::registerLoopCallback,
            "registerLoopCallback__Q29BetterSMS4GameFPCcPFP12TApplication_v");
        KURIBO_EXPORT_AS(BetterSMS::Game::registerPostDrawCallback,
                         "registerPostDrawCallback__Q29BetterSMS4GameFPCcPFP12TApplicationPC13J2DOrthoGraph_v");
        KURIBO_EXPORT_AS(BetterSMS::Game::registerChangeCallback,
                         "registerChangeCallback__Q29BetterSMS4GameFPCcPFP12TApplication_v");
        KURIBO_EXPORT_AS(BetterSMS::Game::deregisterInitCallback,
                         "deregisterInitCallback__Q29BetterSMS4GameFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Game::deregisterBootCallback,
                         "deregisterBootCallback__Q29BetterSMS4GameFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Game::deregisterLoopCallback,
                         "deregisterLoopCallback__Q29BetterSMS4GameFPCc");
        KURIBO_EXPORT_AS(
            BetterSMS::Game::deregisterPostDrawCallback,
            "deregisterPostDrawCallback__Q29BetterSMS4GameFPCc");
        KURIBO_EXPORT_AS(BetterSMS::Game::deregisterChangeCallback,
                         "deregisterChangeCallback__Q29BetterSMS4GameFPCc");

        /* STAGE */
        KURIBO_EXPORT_AS(BetterSMS::Stage::getStageConfiguration,
                         "getStageConfiguration__Q29BetterSMS5StageFv");
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
        KURIBO_EXPORT_AS(BetterSMS::Stage::getStageName, "getStageName__Q29BetterSMS5StageFUcUc");
        KURIBO_EXPORT_AS(BetterSMS::Stage::isExStage, "isExStage__Q29BetterSMS5StageFUcUc");

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

        /* MATH */
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