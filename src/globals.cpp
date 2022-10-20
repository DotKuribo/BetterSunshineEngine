#include <Dolphin/OS.h>
#include <Dolphin/types.h>

#include <SMS/System/Application.hxx>
#include <SMS/macros.h>

#include "collision/warp.hxx"
#include "globals.hxx"
#include "module.hxx"
#include "p_settings.hxx"

SMS_EXTERN_C OSBootInfo BootInfo;

void *BetterSMS::sPRMFile                                                  = nullptr;
BetterSMS::Collision::TWarpCollisionList *BetterSMS::sWarpColArray         = nullptr;
BetterSMS::Collision::TWarpCollisionList *BetterSMS::sWarpColPreserveArray = nullptr;
bool BetterSMS::sIsAudioStreaming                                          = false;
bool BetterSMS::sIsAudioStreamAllowed                                      = false;

#ifdef NDEBUG
bool BetterSMS::sIsDebugMode = true;
#else
bool BetterSMS::sIsDebugMode = true;
#endif

#if BETTER_SMS_WIDESCREEN
u32 BetterSMS::sScreenWidth = 700;
#else
u32 BetterSMS::sScreenWidth = 600;
#endif

f32 BetterSMS::sFrameRate = 30.0f;

extern void initDebugCallbacks(TApplication *app);

bool BetterSMS::isGameEmulated() { return BootInfo.mConsoleType == OS_CONSOLE_DEV_KIT3; }
bool BetterSMS::isDebugMode() { return sIsDebugMode; }
bool BetterSMS::isMusicBeingStreamed() { return sIsAudioStreaming; }
bool BetterSMS::isMusicStreamingAllowed() { return sIsAudioStreamAllowed; }
void BetterSMS::setDebugMode(bool active) {
    const bool isDebugStarted = !sIsDebugMode && active;

	sIsDebugMode = active;

    if (isDebugStarted)
        initDebugCallbacks(&gpApplication);
}

extern AspectRatioSetting gAspectRatioSetting;
int BetterSMS::getScreenWidth() {
    switch (gAspectRatioSetting.getInt()) {
    default:
    case AspectRatioSetting::FULL:
        return 600;
    case AspectRatioSetting::WIDE:
        return 700;
    case AspectRatioSetting::ULTRAWIDE:
        return 1050;
    }
}

f32 BetterSMS::getScreenToFullScreenRatio() { return static_cast<f32>(getScreenWidth()) / 600.0f; }
f32 BetterSMS::getScreenRatioAdjustX() { return (getScreenToFullScreenRatio() - 1.0f) * 600.0f; }

f32 BetterSMS::getFrameRate() { return sFrameRate; }