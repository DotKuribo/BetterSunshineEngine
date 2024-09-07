#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DPane.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/raw_fn.hxx>

#include "debug.hxx"
#include "libs/cheathandler.hxx"
#include "libs/constmath.hxx"
#include "logging.hxx"
#include "module.hxx"
#include "p_debug.hxx"
#include "p_settings.hxx"

using namespace BetterSMS;

static s16 gBaseMonitorX = 500, gBaseMonitorY = 460;

static f32 sFPSTimer       = 0.0f;
static f32 sFPSCounter     = 0.0f;
static OSTime sFPSBaseTime = 0;

static J2DTextBox *gpFPSStringW = nullptr;
static J2DTextBox *gpFPSStringB = nullptr;

static char sStringBuffer[10];
static bool sIsInitialized = false;

BETTER_SMS_FOR_CALLBACK void initFPSMonitor(TApplication *app) {
    gpFPSStringW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpFPSStringB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpFPSStringW->mStrPtr         = sStringBuffer;
    gpFPSStringB->mStrPtr         = sStringBuffer;
    gpFPSStringW->mNewlineSize    = 17;
    gpFPSStringW->mCharSizeX      = 14;
    gpFPSStringW->mCharSizeY      = 17;
    gpFPSStringB->mNewlineSize    = 17;
    gpFPSStringB->mCharSizeX      = 14;
    gpFPSStringB->mCharSizeY      = 17;
    gpFPSStringB->mGradientTop    = {0, 0, 0, 255};
    gpFPSStringB->mGradientBottom = {0, 0, 0, 255};
    sIsInitialized                = true;
}

extern FPSSetting gFPSSetting;

BETTER_SMS_FOR_CALLBACK void updateFPSMonitor(TApplication *app) {
    if (!app->mDirector || !sIsInitialized || !BetterSMS::isDebugMode())
        return;

    sFPSCounter += 1.0f;

    OSTime diff = OSGetTime() - sFPSBaseTime;
    f64 seconds = OSTicksToSeconds(f64(diff));

    if (seconds > 0.5f) {
        const f32 fps = sFPSCounter / seconds;

        snprintf(sStringBuffer, 10, "%.02f FPS", fps);

        int thresholdMultiplier = gFPSSetting.getInt() + 1;

        if (fps < 24.0f * thresholdMultiplier) {
            gpFPSStringW->mGradientTop    = {210, 60, 20, 255};
            gpFPSStringW->mGradientBottom = {210, 60, 20, 255};
        } else if (fps < 29.97f * thresholdMultiplier) {
            gpFPSStringW->mGradientTop    = {130, 170, 10, 255};
            gpFPSStringW->mGradientBottom = {130, 170, 10, 255};
        } else {
            gpFPSStringW->mGradientTop    = {50, 220, 20, 255};
            gpFPSStringW->mGradientBottom = {50, 220, 20, 255};
        }

        sFPSCounter  = 0.0f;
        sFPSBaseTime = OSGetTime();
    }
}

BETTER_SMS_FOR_CALLBACK void drawFPSMonitor(TApplication *app, const J2DOrthoGraph *ortho) {
    if (!sIsInitialized)
        return;

    if (gDebugUIPage == 0 || !BetterSMS::isDebugMode())
        return;

    {
        auto monitorX = gBaseMonitorX + getScreenRatioAdjustX();
        gpFPSStringB->draw(monitorX + 1, gBaseMonitorY + 2);
        gpFPSStringW->draw(monitorX, gBaseMonitorY);
    }
}