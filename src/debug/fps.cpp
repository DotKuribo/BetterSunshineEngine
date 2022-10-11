#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DPane.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/game/Application.hxx>
#include <SMS/npc/BaseNPC.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/sound/MSound.hxx>
#include <SMS/sound/MSoundSESystem.hxx>

#include "debug.hxx"
#include "libs/cheathandler.hxx"
#include "libs/constmath.hxx"
#include "logging.hxx"
#include "module.hxx"

using namespace BetterSMS;

#if BETTER_SMS_WIDESCREEN
static s16 gMonitorX = 390, gMonitorY = 460;
#else
static s16 gMonitorX = 390, gMonitorY = 460;
#endif

static f32 sFPSTimer       = 0.0f;
static f32 sFPSCounter     = 0.0f;
static OSTime sFPSBaseTime = 0;

static J2DTextBox *gpFPSStringW = nullptr;
static J2DTextBox *gpFPSStringB = nullptr;

static char sStringBuffer[10];
static bool sIsInitialized = false;

void initFPSMonitor(TApplication *app) {
    gpFPSStringW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpFPSStringB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpFPSStringW->mStrPtr         = sStringBuffer;
    gpFPSStringB->mStrPtr         = sStringBuffer;
    gpFPSStringW->mNewlineSize    = 18;
    gpFPSStringW->mCharSizeX      = 15;
    gpFPSStringW->mCharSizeY      = 18;
    gpFPSStringB->mNewlineSize    = 18;
    gpFPSStringB->mCharSizeX      = 15;
    gpFPSStringB->mCharSizeY      = 18;
    gpFPSStringB->mGradientTop    = {0, 0, 0, 255};
    gpFPSStringB->mGradientBottom = {0, 0, 0, 255};
    sIsInitialized                = true;
}

void updateFPSMonitor(TApplication *app) {
    if (!app->mDirector || !sIsInitialized)
        return;

    sFPSCounter += 1.0f;

    OSTime diff = OSGetTime() - sFPSBaseTime;
    f64 seconds = OSTicksToSeconds(f64(diff));

    if (seconds > 0.5f) {
        const f32 fps = sFPSCounter / seconds;

        snprintf(sStringBuffer, 10, "%.02f FPS", fps);

        if (fps < 24.0f) {
            gpFPSStringW->mGradientTop    = {210, 60, 20, 255};
            gpFPSStringW->mGradientBottom = {210, 60, 20, 255};
        } else if (fps < 29.97f) {
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

void drawFPSMonitor(TApplication *app, J2DOrthoGraph *ortho) {
    if (!sIsInitialized)
        return;

    gpFPSStringB->draw(gMonitorX + 1, gMonitorY + 2);
    gpFPSStringW->draw(gMonitorX, gMonitorY);
}