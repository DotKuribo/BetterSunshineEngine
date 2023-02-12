#pragma once

#ifndef HW_DOL
#define HW_DOL
#endif

#include <Dolphin/types.h>
#include <SMS/macros.h>
#include <sdk.h>

#define BETTER_SMS_MODULE_NAME "BetterSunshineEngine"
#define BETTER_SMS_AUTHOR_NAME "JoshuaMK"

#ifndef BETTER_SMS_VERSION
#define BETTER_SMS_VERSION "(unknown version)"
#endif

#ifndef BETTER_SMS_MAX_SHINES
#define BETTER_SMS_MAX_SHINES 120
#else
#if BETTER_SMS_MAX_SHINES < 1
#undef BETTER_SMS_MAX_SHINES
#define BETTER_SMS_MAX_SHINES 1
#elif BETTER_SMS_MAX_SHINES > 999
#undef BETTER_SMS_MAX_SHINES
#define BETTER_SMS_MAX_SHINES 999
#endif
#endif

/* Misc compiler info */
#ifndef BETTER_SMS_VERSION
#define BETTER_SMS_VERSION "vUnknown"
#endif

#ifdef __VERSION__
#define BETTER_SMS_CC_VERSION __VERSION__
#elif defined(__CWCC__)
#define BETTER_SMS_CC_VERSION "CWCC " STRINGIZE(__CWCC__)
#else
#define BETTER_SMS_CC_VERSION "Unknown"
#endif

#if SMS_DEBUG
#define BETTER_SMS_VERSION_TAG                                                                     \
    "(DEBUG) " BETTER_SMS_VERSION "[" SMS_STRINGIZE(BETTER_SMS_MAX_SHINES) " Shines]"
#else
#define BETTER_SMS_VERSION_TAG                                                                     \
    "(RELEASE) " BETTER_SMS_VERSION "[" SMS_STRINGIZE(BETTER_SMS_MAX_SHINES) " Shines]"
#endif

/* CONFIGURATION DEFINES */

#ifndef BETTER_SMS_BUGFIXES
#define BETTER_SMS_BUGFIXES 0
#endif

#ifndef BETTER_SMS_CRASHFIXES
#define BETTER_SMS_CRASHFIXES 0
#endif

#ifndef BETTER_SMS_EXCEPTION_HANDLER
#define BETTER_SMS_EXCEPTION_HANDLER 0
#endif

#ifndef BETTER_SMS_SPC_LOGGING
#define BETTER_SMS_SPC_LOGGING 0
#endif

#ifndef BETTER_SMS_SLOT_B_SUPPORT
#define BETTER_SMS_SLOT_B_SUPPORT 0
#endif

#ifndef BETTER_SMS_EXTENDED_RENDER_DISTANCE
#define BETTER_SMS_EXTENDED_RENDER_DISTANCE 0
#endif

#ifndef BETTER_SMS_WIDESCREEN
#define BETTER_SMS_WIDESCREEN 0
#endif

#ifndef BETTER_SMS_CUSTOM_MUSIC
#define BETTER_SMS_CUSTOM_MUSIC 0
#endif

#ifndef BETTER_SMS_EXTRA_SHINES
#define BETTER_SMS_EXTRA_SHINES 0
#endif

#ifndef BETTER_SMS_EXTRA_OBJECTS
#define BETTER_SMS_EXTRA_OBJECTS 0
#endif

#ifndef BETTER_SMS_EXTRA_COLLISION
#define BETTER_SMS_EXTRA_COLLISION 0
#endif

#ifndef BETTER_SMS_EXTRA_BMG_COMMANDS
#define BETTER_SMS_EXTRA_BMG_COMMANDS 0
#endif

#ifndef BETTER_SMS_NO_TITLE_THP
#define BETTER_SMS_NO_TITLE_THP 0
#endif

#ifndef BETTER_SMS_SWAP_LZ_BUTTONS
#define BETTER_SMS_SWAP_LZ_BUTTONS 0
#endif

#ifndef BETTER_SMS_LONG_JUMP
#define BETTER_SMS_LONG_JUMP 0
#endif

#ifndef BETTER_SMS_MULTI_JUMP
#define BETTER_SMS_MULTI_JUMP 0
#endif

#ifndef BETTER_SMS_HOVER_BURST
#define BETTER_SMS_HOVER_BURST 0
#endif

#ifndef BETTER_SMS_HOVER_SLIDE
#define BETTER_SMS_HOVER_SLIDE 0
#endif

#ifndef BETTER_SMS_ROCKET_DIVE
#define BETTER_SMS_ROCKET_DIVE 0
#endif

#ifndef BETTER_SMS_DYNAMIC_FALL_DAMAGE
#define BETTER_SMS_DYNAMIC_FALL_DAMAGE 0
#endif

#ifndef BETTER_SMS_NO_DOWNWARP
#define BETTER_SMS_NO_DOWNWARP 0
#endif

#ifndef BETTER_SMS_SHADOW_MARIO_HEALTHBAR
#define BETTER_SMS_SHADOW_MARIO_HEALTHBAR 0
#endif

#ifndef BETTER_SMS_GREEN_YOSHI
#define BETTER_SMS_GREEN_YOSHI 0
#endif

#ifndef BETTER_SMS_YOSHI_SAVE_NOZZLES
#define BETTER_SMS_YOSHI_SAVE_NOZZLES 0
#endif

#ifndef BETTER_SMS_UNDERWATER_FRUIT
#define BETTER_SMS_UNDERWATER_FRUIT 0
#endif

#ifndef BETTER_SMS_USE_PS_MATH
#define BETTER_SMS_USE_PS_MATH 1
#endif

namespace BetterSMS {

    namespace Settings {
        class SettingsGroup;
    }

    struct ModuleInfo {
        const char *mName;
        u16 mVersionMajor;
        u16 mVersionMinor;
        Settings::SettingsGroup *mSettings;
    };

    bool isGameEmulated();
    bool isMusicBeingStreamed();
    bool isMusicStreamingAllowed();

    bool isDebugMode();
    void setDebugMode(bool);

    const ModuleInfo *getModuleInfo(const char *key);

    bool isModuleRegistered(const char *key);
    bool registerModule(const ModuleInfo *info);
    bool deregisterModule(const char *key);

    int getScreenRenderWidth();
    int getScreenOrthoWidth();
    f32 getScreenToFullScreenRatio();
    f32 getScreenRatioAdjustX();
    f32 getFrameRate();
}  // namespace BetterSMS