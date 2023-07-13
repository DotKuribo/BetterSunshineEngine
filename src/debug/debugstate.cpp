#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DPane.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Player/Mario.hxx>

#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/NPC/NpcBase.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/raw_fn.hxx>

#include "debug.hxx"
#include "libs/cheathandler.hxx"
#include "libs/constmath.hxx"
#include "libs/profiler.hxx"
#include "logging.hxx"
#include "module.hxx"
#include "p_debug.hxx"

using namespace BetterSMS;

static s16 gMonitorX = 10, gMonitorY = 315;
static s16 gFontWidth = 11, gFontHeight = 11;

static J2DTextBox *gpDebugStateStringW   = nullptr;
static J2DTextBox *gpDebugStateStringB   = nullptr;
static J2DTextBox *gpDebugControlStringW = nullptr;
static J2DTextBox *gpDebugControlStringB = nullptr;

static char sDebugStateStringBuffer[30]{};
static char sDebugControlStringBuffer[400]{};

static const char *sDebugModeNames[]    = {"Mario XYZ", "Free Camera"};
static const char *sButtonDescriptors[] = {
    "D-Left", "D-Right", "D-Down",  "D-Up",
    "\x24",  // Z
    "\x3E",  // R
    "\x3C",  // L
    "\x40",  // A
    "\x23",  // B
    "\x2B",  // X
    "\xA5",  // Y
    "Start",  "C-Left",  "C-Right", "C-Down", "C-Up", "M-Left", "M-Right", "M-Down", "M-Up",
};

static const char *getStringOfInput(int input) {
    int index = 0;

    while (input > 1) {
        input >>= 1;
        if (input == 0x80 || input == 0x2000 || input == 0x4000 || input == 0x8000 ||
            input > 0x8000000)
            continue;
        index += 1;
    }

    return sButtonDescriptors[index];
}

static bool sIsInitialized = false;

BETTER_SMS_FOR_CALLBACK void initDebugStateMonitor(TApplication *app) {
    gpDebugStateStringW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpDebugStateStringB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpDebugStateStringW->mStrPtr         = sDebugStateStringBuffer;
    gpDebugStateStringB->mStrPtr         = sDebugStateStringBuffer;
    gpDebugStateStringW->mGradientTop    = {50, 255, 50, 255};
    gpDebugStateStringW->mGradientBottom = {255, 255, 50, 255};
    gpDebugStateStringB->mGradientTop    = {0, 0, 0, 255};
    gpDebugStateStringB->mGradientBottom = {0, 0, 0, 255};

    gpDebugControlStringW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpDebugControlStringB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpDebugControlStringW->mStrPtr         = sDebugControlStringBuffer;
    gpDebugControlStringB->mStrPtr         = sDebugControlStringBuffer;
    gpDebugControlStringW->mNewlineSize    = gFontHeight;
    gpDebugControlStringW->mCharSizeX      = gFontWidth;
    gpDebugControlStringW->mCharSizeY      = gFontHeight;
    gpDebugControlStringB->mNewlineSize    = gFontHeight;
    gpDebugControlStringB->mCharSizeX      = gFontWidth;
    gpDebugControlStringB->mCharSizeY      = gFontHeight;
    gpDebugControlStringW->mGradientTop    = {255, 255, 255, 255};
    gpDebugControlStringW->mGradientBottom = {255, 255, 255, 255};
    gpDebugControlStringB->mGradientTop    = {0, 0, 0, 255};
    gpDebugControlStringB->mGradientBottom = {0, 0, 0, 255};

    sIsInitialized = true;
}

BETTER_SMS_FOR_CALLBACK void updateDebugStateMonitor(TApplication *app) {
    TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

    if (app->mContext != TApplication::CONTEXT_DIRECT_STAGE || !gpMarioAddress || !sIsInitialized)
        return;

    if (director->mCurState == TMarDirector::STATE_INTRO_INIT)
        return;

    if (gDebugState == NONE) {
        snprintf(sDebugStateStringBuffer, 7, "%s", "(GAME)");

        snprintf(sDebugControlStringBuffer, 400,
                 "Controls:\n"
                 "  Toggle Debug UI:  (%s) | %s\n"
                 "  Toggle Game UI:    %s\n"
                 "  Toggle Mode:      (%s) | %s\n"
                 "  Change Nozzle:     %s or %s",
                 getStringOfInput(gSecondaryMask), getStringOfInput(gControlToggleDebugUI),
                 getStringOfInput(gControlToggleGameUI), getStringOfInput(gSecondaryMask),
                 getStringOfInput(gControlToggleDebugState),
                 getStringOfInput(gControlNozzleSwitchL), getStringOfInput(gControlNozzleSwitchR));
    } else if (gDebugState == XYZ_MODE) {
        snprintf(sDebugStateStringBuffer, 6, "%s", "(XYZ)");

        snprintf(sDebugControlStringBuffer, 400,
                 "Controls:\n"
                 "  Toggle Mode:        (%s) | %s\n"
                 "  Toggle Debug UI:    (%s) | %s\n"
                 "  Toggle Game UI:      %s\n"
                 "  Toggle Fast Mode:   %s\n"
                 "  Change Animation:    %s or %s\n"
                 "  Change Anm Speed: (%s) %s or %s\n"
                 "  Move XZ:             Control Stick\n"
                 "  Move Y:              L or R\n",
                 getStringOfInput(gSecondaryMask), getStringOfInput(gControlToggleDebugState),
                 getStringOfInput(gSecondaryMask), getStringOfInput(gControlToggleDebugUI),
                 getStringOfInput(gControlToggleGameUI),
                 getStringOfInput(gControlToggleFastMovement),
                 getStringOfInput(gControlDecreaseAnimationID),
                 getStringOfInput(gControlIncreaseAnimationID), getStringOfInput(gSecondaryMask),
                 getStringOfInput(gControlDecreaseAnimationSpeed),
                 getStringOfInput(gControlIncreaseAnimationSpeed));
    } else if (gDebugState == CAM_MODE) {
        snprintf(sDebugStateStringBuffer, 9, "%s", "(CAMERA)");

        snprintf(sDebugControlStringBuffer, 400,
                 "Controls:\n"
                 "  Toggle Mode:        (%s) | %s\n"
                 "  Toggle Debug UI:    (%s) | %s\n"
                 "  Toggle Game UI:      %s\n"
                 "  Toggle Fast Mode:   %s\n"
                 "  Change Animation:    %s or %s\n"
                 "  Change Anm Speed: (%s) %s or %s\n"
                 "  Move XZ:             Control Stick\n"
                 "  Move Y:              L or R\n"
                 "  Pitch/Yaw:           C Stick\n"
                 "  Roll:                 (%s) | L or R\n"
                 "  Change FOV:        (%s) | C Stick Up/Down\n",
                 getStringOfInput(gSecondaryMask), getStringOfInput(gControlToggleDebugState),
                 getStringOfInput(gSecondaryMask), getStringOfInput(gControlToggleDebugUI),
                 getStringOfInput(gControlToggleGameUI),
                 getStringOfInput(gControlToggleFastMovement),
                 getStringOfInput(gControlDecreaseAnimationID),
                 getStringOfInput(gControlIncreaseAnimationID), getStringOfInput(gSecondaryMask),
                 getStringOfInput(gControlDecreaseAnimationSpeed),
                 getStringOfInput(gControlIncreaseAnimationSpeed), getStringOfInput(gSecondaryMask),
                 getStringOfInput(gSecondaryMask));
    }
}

BETTER_SMS_FOR_CALLBACK void drawDebugStateMonitor(TApplication *app, const J2DOrthoGraph *ortho) {
    TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

    if (app->mContext != TApplication::CONTEXT_DIRECT_STAGE || !gpMarioAddress || !sIsInitialized)
        return;

    if (director->mCurState == TMarDirector::STATE_INTRO_INIT)
        return;

    if (gDebugUIPage == 0)
        return;

    s16 adjust = getScreenRatioAdjustX();
    gpDebugStateStringB->draw(381, 462);
    gpDebugStateStringW->draw(380, 460);
    gpDebugControlStringB->draw(gMonitorX - adjust + 1, gMonitorY + 1);
    gpDebugControlStringW->draw(gMonitorX - adjust, gMonitorY);
}