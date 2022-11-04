#include <Dolphin/OS.h>
#include <Dolphin/types.h>

#include <SMS/System/Application.hxx>
#include <SMS/macros.h>

#include "collision/warp.hxx"
#include "p_globals.hxx"
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
    case AspectRatioSetting::FULL:
        return 600;
    case AspectRatioSetting::WIDE:
        return 700;
    case AspectRatioSetting::ULTRAWIDE:
        return 1050;
    }
}
int BetterSMS::getScreenOrthoWidth() {
    switch (gAspectRatioSetting.getInt()) {
    default:
    case AspectRatioSetting::FULL:
        return 640;
    case AspectRatioSetting::WIDE:
        return 853;
    case AspectRatioSetting::ULTRAWIDE:
        return 1120;
    }
}

f32 BetterSMS::getScreenToFullScreenRatio() { return static_cast<f32>(getScreenRenderWidth()) / 600.0f; }
f32 BetterSMS::getScreenRatioAdjustX() { return (getScreenToFullScreenRatio() - 1.0f) * 600.0f; }

f32 BetterSMS::getFrameRate() {
    switch (gFPSSetting.getInt()) {
    default:
    case FPSSetting::FPS_30:
        return SMS_PORT_REGION(30.0f, 25.0f, 30.0f, 30.0f);
    case FPSSetting::FPS_60:
        return SMS_PORT_REGION(60.0f, 50.0f, 60.0f, 60.0f);
    case FPSSetting::FPS_120:
        return SMS_PORT_REGION(120.0f, 100.0f, 120.0f, 120.0f);
    }
}