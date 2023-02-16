#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>

#include "debug.hxx"
#include "libs/cheathandler.hxx"
#include "libs/constmath.hxx"
#include "logging.hxx"
#include "module.hxx"

using namespace BetterSMS;

/* CHEAT HANDLER */

static u16 gDebugModeCheatCode[] = {
    TMarioGamePad::DPAD_UP,   TMarioGamePad::DPAD_UP,    TMarioGamePad::DPAD_DOWN,
    TMarioGamePad::DPAD_DOWN, TMarioGamePad::DPAD_LEFT,  TMarioGamePad::DPAD_RIGHT,
    TMarioGamePad::DPAD_LEFT, TMarioGamePad::DPAD_RIGHT, TMarioGamePad::B,
    TMarioGamePad::A,         TMarioGamePad::START};

J2DTextBox *gDebugTextBoxW;
J2DTextBox *gDebugTextBoxB;
TCheatHandler gDebugHandler;

static void debugModeNotify(TCheatHandler *) {
    if (gpMSound->gateCheck(MSound::SE_SHINE_TOUCH)) {
        auto *sound =
            MSoundSESystem::MSoundSE::startSoundSystemSE(MSound::SE_NINTENDO_SOUND, 0, 0, 0);
        if (sound)
            sound->setPitch(1.25f, 0, 0);
    }

    BetterSMS::setDebugMode(true);
    Console::debugLog("Debug mode activated!\n");
}

// extern -> debug callback
void drawCheatText(TApplication *app, const J2DOrthoGraph *graph) {
    if (!gDebugTextBoxW || !gDebugTextBoxW->getStringPtr())
        return;

    if (BetterSMS::isDebugMode()) {
        gDebugTextBoxB->draw(235, 462);
        gDebugTextBoxW->draw(234, 460);
    }
}

static void *handleDebugCheat(void *GCLogoDir) {
    if (!gDebugHandler.isInitialized()) {
        gDebugHandler.setGamePad(gpApplication.mGamePads[0]);
        gDebugHandler.setInputList(gDebugModeCheatCode);
        gDebugHandler.setSuccessCallBack(&debugModeNotify);

        auto *currentHeap               = JKRHeap::sRootHeap->becomeCurrentHeap();
        gDebugTextBoxW                  = new J2DTextBox(gpSystemFont->mFont, "Debug Mode");
        gDebugTextBoxB                  = new J2DTextBox(gpSystemFont->mFont, "Debug Mode");
        gDebugTextBoxW->mGradientTop    = {255, 50, 50, 255};
        gDebugTextBoxW->mGradientBottom = {255, 50, 255, 255};
        gDebugTextBoxB->mGradientTop    = {0, 0, 0, 255};
        gDebugTextBoxB->mGradientBottom = {0, 0, 0, 255};
        currentHeap->becomeCurrentHeap();

        BetterSMS::Debug::registerDrawCallback("__better_sms_draw_debug", drawCheatText);
    }
    gDebugHandler.advanceInput();
    return GCLogoDir;
}
SMS_PATCH_B(SMS_PORT_REGION(0x80295B6C, 0x8028DA04, 0, 0), handleDebugCheat);

/* DEBUG MODS */

static void isLevelSelectAvailable() {
    u32 context;
    SMS_FROM_GPR(30, context);

    if (context == 9 || context == 4)
        context = BetterSMS::isDebugMode() ? 9 : 4;
    gpApplication.mContext = context;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802A6794, 0x8029E6EC, 0, 0), isLevelSelectAvailable);

u32 XYZState = 0xF000FFFF;
static bool sJustStartedXYZ = false;
static bool sJustExitedXYZ  = false;

BETTER_SMS_FOR_CALLBACK void checkMarioXYZMode(TMario *player, bool isMario) {
    constexpr u32 enterButton = TMarioGamePad::EButtons::DPAD_UP;

    if ((player->mController->mButtons.mInput & TMarioGamePad::L))
        return;

    if (player->mState == XYZState)
        return;

    if ((player->mController->mButtons.mFrameInput & enterButton) && BetterSMS::isDebugMode()) {
        if (!sJustExitedXYZ) {
            player->mPrevState = player->mState;
            player->mState = XYZState;
            sJustStartedXYZ = true;
        }
    } else {
        sJustExitedXYZ = false;
    }
}

// extern -> debug update callback
BETTER_SMS_FOR_CALLBACK bool updateMarioXYZMode(TMario *player) {
    constexpr f32 baseSpeed = 21.0f;
    constexpr u32 exitButton = TMarioGamePad::EButtons::DPAD_UP;

    player->mSpeed.set(0.0f, 0.0f, 0.0f);
    player->mForwardSpeed = 0.0f;

    if ((player->mController->mButtons.mFrameInput & exitButton)) {
        if (!sJustStartedXYZ && !(player->mController->mButtons.mInput & TMarioGamePad::L)) {
            player->mState = TMario::STATE_FALL;
            sJustExitedXYZ = true;
            return true;
        }
    } else {
        sJustStartedXYZ = false;
    }

    const JUTGamePad::CStick &mainStick = player->mController->mControlStick;
    const f32 speedMultiplier           = lerp<f32>(1, 2, player->mController->mButtons.mAnalogR);

    const f32 cameraRotY = (f32)(gpCamera->mHorizontalAngle) / 182.0f;

    player->mTranslation.x +=
        ((-sinf(angleToRadians(cameraRotY)) * baseSpeed) * speedMultiplier) * mainStick.mStickY;
    player->mTranslation.z +=
        ((-cosf(angleToRadians(cameraRotY)) * baseSpeed) * speedMultiplier) * mainStick.mStickY;
    player->mTranslation.x -= ((-sinf(angleToRadians(cameraRotY + 90.0f)) * baseSpeed) * speedMultiplier) *
                    mainStick.mStickX;
    player->mTranslation.z -= ((-cosf(angleToRadians(cameraRotY + 90.0f)) * baseSpeed) * speedMultiplier) *
                    mainStick.mStickX;

    if (player->mController->mButtons.mInput & TMarioGamePad::EButtons::B) {
        player->mTranslation.y -= (baseSpeed * speedMultiplier);
    } else if (player->mController->mButtons.mInput & TMarioGamePad::EButtons::A) {
        player->mTranslation.y += (baseSpeed * speedMultiplier);
    }
    return false;
}

enum DebugNozzleKind {
    SPRAY,
    ROCKET,
    UNDERWATER,
    YOSHI,
    HOVER,
    TURBO
};

static s32 sNozzleKind = DebugNozzleKind::SPRAY;

// extern -> debug update callback
BETTER_SMS_FOR_CALLBACK void updateFluddNozzle(TApplication *app) {
    TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

    if (!director || !BetterSMS::isDebugMode() ||
        director->mCurState != TMarDirector::Status::STATE_NORMAL)
        return;

    TMario *player = gpMarioAddress;
    TWaterGun *fludd = player->mFludd;

    if (!fludd)
        return;

    sNozzleKind = fludd->mSecondNozzle;

    s32 adjust = 0;
    if ((player->mController->mButtons.mFrameInput & TMarioGamePad::DPAD_RIGHT))
        adjust = 1;
    else if ((player->mController->mButtons.mFrameInput & TMarioGamePad::DPAD_LEFT))
        adjust = -1;

    if (adjust == 0)
        return;

    player->mAttributes.mHasFludd = true;

    switch (sNozzleKind) {
    case SPRAY:
        if (adjust > 0)
            fludd->mSecondNozzle = TWaterGun::Rocket;
        else if (adjust < 0)
            fludd->mSecondNozzle = TWaterGun::Turbo;
        break;
    case ROCKET:
        fludd->mSecondNozzle = TWaterGun::Rocket + adjust;
        break;
    case UNDERWATER:
        fludd->mSecondNozzle = TWaterGun::Underwater + adjust;
        break;
    case YOSHI:
        fludd->mSecondNozzle = TWaterGun::Yoshi + adjust;
        break;
    case HOVER:
        fludd->mSecondNozzle = TWaterGun::Hover + adjust;
        break;
    case TURBO:
        fludd->mSecondNozzle = TWaterGun::Turbo + adjust;
        if (adjust > 0)
            fludd->mSecondNozzle = TWaterGun::Spray;
        else if (adjust < 0)
            fludd->mSecondNozzle = TWaterGun::Hover;
        break;
    }

    fludd->mCurrentNozzle = fludd->mSecondNozzle;

    if (sNozzleKind != fludd->mSecondNozzle) {
        fludd->mCurrentNozzle = fludd->mSecondNozzle;
        TNozzleBase *currentNozzle = fludd->mNozzleList[fludd->mCurrentNozzle];
        fludd->mCurrentWater       = currentNozzle->mEmitParams.mAmountMax.get();
    }

    sNozzleKind %= 7;
}

// static u8 sHomeID   = 0;
// static u8 sTargetID = 254;
// static TBGCheckData *sHomeTriangle;

// void updateDebugCollision(TMario *player) {
//     constexpr u32 SetHomeTriangleButtons =
//         TMarioGamePad::EButtons::Z | TMarioGamePad::EButtons::DPAD_LEFT;
//     constexpr u32 SetTargetTriangleButtons =
//         TMarioGamePad::EButtons::Z | TMarioGamePad::EButtons::DPAD_RIGHT;

//     if (!TGlobals::isDebugMode())
//         return;

//     const JUTGamePad::CButton &buttons = player->mController->mButtons;

//     if (buttons.mFrameInput == SetHomeTriangleButtons) {
//         sHomeTriangle = player->mFloorTriangle;
//         sHomeID       = (sHomeID + 1) % 255;
//     } else if (buttons.mFrameInput == SetTargetTriangleButtons && sHomeTriangle) {
//         if (sHomeTriangle == player->mFloorTriangle)
//             return;

//         sHomeTriangle->mType          = 16042;
//         sHomeTriangle->mValue                 = s16((sTargetID << 8) | (sHomeID - 1));
//         player->mFloorTriangle->mType = 16042;
//         player->mFloorTriangle->mValue        = s16(((sHomeID - 1) << 8) | sTargetID);
//         TGlobals::sWarpColArray->addLink(sHomeTriangle, player->mFloorTriangle);
//         TGlobals::sWarpColArray->addLink(player->mFloorTriangle, sHomeTriangle);
//         sTargetID = sTargetID != 0 ? (sTargetID - 1) : 254;
//     }

//     return;
// }

// static void setEmitPrm() {
//     TWaterBalloon::sEmitInfo = new TWaterEmitInfo("/mario/waterballoon/waterballoon.prm");
//     TParams::finalize();
// }
// SMS_PATCH_BL(SMS_PORT_REGION(0x802B8DC8, 0x802B0D98, 0, 0), setEmitPrm);

static u32 preventDebuggingDeath(TMario *player) {
    return player->mState == XYZState ? 0x224E0 : player->mState;  // Spoof non dying value
};
//SMS_PATCH_BL(SMS_PORT_REGION(0x80243110, 0x8023AE9C, 0, 0), preventDebuggingDeath);

static void preventDebuggingInteraction(TMario *player, JDrama::TGraphics *graphics) {
    if (player->mState != XYZState)
        player->playerControl(graphics);
    else {
        player->mForwardSpeed = 0.0f;
        player->mSpeed        = {0.0f, 0.0f, 0.0f};
        player->mState        = static_cast<u32>(TMario::STATE_IDLE);
    }
}
//SMS_PATCH_BL(SMS_PORT_REGION(0x8024D3A0, 0x8024512C, 0, 0), preventDebuggingInteraction);