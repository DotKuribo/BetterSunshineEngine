#include <Dolphin/OS.h>
#include <Dolphin/types.h>

#include <SMS/System/Application.hxx>
#include <SMS/macros.h>

#include "module.hxx"
#include "p_globals.hxx"
#include "p_settings.hxx"
#include "p_warp.hxx"

SMS_EXTERN_C OSBootInfo BootInfo;

void *BetterSMS::sPRMFile                                                  = nullptr;
BetterSMS::Collision::TWarpCollisionList *BetterSMS::sWarpColArray         = nullptr;
BetterSMS::Collision::TWarpCollisionList *BetterSMS::sWarpColPreserveArray = nullptr;
bool BetterSMS::sIsAudioStreaming                                          = false;

#if SMS_DEBUG
bool BetterSMS::sIsDebugMode = true;
#else
bool BetterSMS::sIsDebugMode = false;
#endif

extern void initDebugCallbacks(TApplication *app);

bool BetterSMS::isGameEmulated() { return BootInfo.mConsoleType == OS_CONSOLE_DEV_KIT3; }
bool BetterSMS::isDebugMode() { return sIsDebugMode; }
bool BetterSMS::isMusicBeingStreamed() { return sIsAudioStreaming; }
bool BetterSMS::isMusicStreamingAllowed() { return *reinterpret_cast<bool *>(0x80000008); }
void BetterSMS::setDebugMode(bool active) {
    const bool isDebugStarted = !sIsDebugMode && active;

    sIsDebugMode = active;

    if (isDebugStarted)
        initDebugCallbacks(&gpApplication);
}

extern AspectRatioSetting gAspectRatioSetting;
extern FPSSetting gFPSSetting;

int BetterSMS::getScreenRenderWidth() {
    switch (gAspectRatioSetting.getInt()) {
    default:
    case AspectRatioSetting::FULLOPENMATTE:
        return 600;
    case AspectRatioSetting::FULL:
        return 600;
    case AspectRatioSetting::STEAMDECK:
        return 716;
    case AspectRatioSetting::WIDE:
        return 796;
    case AspectRatioSetting::ULTRAWIDE:
        return 1045;
    case AspectRatioSetting::STUPIDWIDE:
        return 1583;
    }
}
int BetterSMS::getScreenOrthoWidth() {
    switch (gAspectRatioSetting.getInt()) {
    default:
    case AspectRatioSetting::FULLOPENMATTE:
        return 640;
    case AspectRatioSetting::FULL:
        return 640;
    case AspectRatioSetting::STEAMDECK:
        return 768;
    case AspectRatioSetting::WIDE:
        return 853;
    case AspectRatioSetting::ULTRAWIDE:
        return 1120;
    case AspectRatioSetting::STUPIDWIDE:
        return 1699;
    }
}

f32 BetterSMS::getScreenToFullScreenRatio() {
    return static_cast<f32>(getScreenRenderWidth()) / 600.0f;
}
f32 BetterSMS::getScreenRatioAdjustX() { return (getScreenToFullScreenRatio() - 1.0f) * 300.0f; }

f32 BetterSMS::getFrameRate() {
    const f32 FPS = static_cast<f32>(30 << gFPSSetting.getInt());

    return SMS_PORT_REGION(FPS, FPS / 1.2, FPS, FPS);
}

extern BugsSetting gBugFixesSetting;
extern BugsSetting gExploitFixesSetting;
extern CollisionFixesSetting gCollisionFixesSetting;

BugsSetting *BetterSMS::getBugFixesSetting() { return &gBugFixesSetting; }
BugsSetting *BetterSMS::getExploitFixesSetting() { return &gExploitFixesSetting; }
CollisionFixesSetting *BetterSMS::getCollisionFixesSetting() { return &gCollisionFixesSetting; }