#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/MarioUtil/MathUtil.hxx>

#include "debug.hxx"
#include "libs/cheathandler.hxx"
#include "libs/constmath.hxx"
#include "libs/geometry.hxx"
#include "logging.hxx"
#include "module.hxx"
#include "raw_fn.hxx"

#include "p_debug.hxx"
#include <player.hxx>

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

// extern -> debug callback
void drawCheatText(TApplication *app, const J2DOrthoGraph *graph) {
    if (!gDebugTextBoxW || !gDebugTextBoxW->getStringPtr())
        return;

    if (BetterSMS::isDebugMode() && gDebugUIPage != 0) {
        gDebugTextBoxB->draw(235, 462);
        gDebugTextBoxW->draw(234, 460);
    }
}

static void debugModeNotify(TCheatHandler *) {
    if (gpMSound->gateCheck(MSD_SE_MV_CHAO)) {
        auto *sound = MSoundSESystem::MSoundSE::startSoundSystemSE(MSD_SE_MV_CHAO, 0, 0, 0);
        if (sound)
            sound->setPitch(1.25f, 0, 0);
    }

    BetterSMS::Debug::addDrawCallback(drawCheatText);
    BetterSMS::setDebugMode(true);
}

static void *handleDebugCheat(void *GCLogoDir) {
    if (!gDebugHandler.isInitialized()) {
        gDebugHandler.setGamePad(gpApplication.mGamePads[0]);
#if SMS_DEBUG
        gDebugHandler.succeed();
        BetterSMS::Debug::addDrawCallback(drawCheatText);
#else
        gDebugHandler.setGamePad(gpApplication.mGamePads[0]);
        gDebugHandler.setInputList(gDebugModeCheatCode);
        gDebugHandler.setSuccessCallBack(&debugModeNotify);
#endif
        auto *currentHeap               = JKRHeap::sRootHeap->becomeCurrentHeap();
        gDebugTextBoxW                  = new J2DTextBox(gpSystemFont->mFont, "Debug Mode");
        gDebugTextBoxB                  = new J2DTextBox(gpSystemFont->mFont, "Debug Mode");
        gDebugTextBoxW->mGradientTop    = {255, 50, 50, 255};
        gDebugTextBoxW->mGradientBottom = {255, 50, 255, 255};
        gDebugTextBoxB->mGradientTop    = {0, 0, 0, 255};
        gDebugTextBoxB->mGradientBottom = {0, 0, 0, 255};
        currentHeap->becomeCurrentHeap();
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

bool gIsGameUIActive  = true;
int gMarioAnimation   = 0;
bool gIsDebugActive   = false;
bool gIsFastMovement  = false;
int gAnimationID      = 0;
float gAnimationSpeed = 1.0f;

static bool sJustStartedDebug = false;
static bool sJustExitedDebug  = false;
static int sNumAnimations     = 336;

extern void DebugMoveMarioXYZ(TMario *player);
extern void DebugLookAtCamera(TMario *player, CPolarSubCamera *camera);
extern void DebugCamera(TMario *player, CPolarSubCamera *camera);

//----//

BETTER_SMS_FOR_CALLBACK void checkDebugMode(TMario *player, bool isMario) {
    // Just toggle the global things here.
    if (!isMario) {
        return;
    }

    const bool shouldToggleDebugUI =
        ButtonsFramePressed(player->mController, gControlToggleDebugUI) &&
        ButtonsPressed(player->mController, gSecondaryMask);

    const bool shouldToggleGameUI =
        ButtonsFramePressed(player->mController, gControlToggleGameUI) &&
        !ButtonsPressed(player->mController, gSecondaryMask);

    if (shouldToggleDebugUI) {
        gDebugUIPage = (gDebugUIPage + 1) % 6;
    }

    if (shouldToggleGameUI && BetterSMS::isDebugMode()) {
        gIsGameUIActive ^= true;
    }

    // If the camera is frozen and the tracking is active, we need to update the camera
    if (gIsCameraFrozen && gDebugState != CAM_MODE) {
        if (gIsCameraTracking) {
            DebugLookAtCamera(player, gpCamera);
        } else {
            gpCamera->mAnglePitch = convertAngleFloatToS16(gCamRotation.x);
            gpCamera->mAngleYaw   = convertAngleFloatToS16(180.0f + gCamRotation.y);
        }
    }

    if (player->mState == XYZState)
        return;

    gIsDebugActive = false;
    gDebugState    = NONE;

    if (ButtonsFramePressed(player->mController, gControlToggleDebugActive) &&
        ButtonsPressed(player->mController, gActivateMask) && BetterSMS::isDebugMode()) {
        if (!sJustExitedDebug) {
            player->mPrevState = player->mState;
            player->mState     = XYZState;
            sJustStartedDebug  = true;
        }
    } else {
        sJustExitedDebug = false;
    }
}

// extern -> debug update callback
BETTER_SMS_FOR_CALLBACK bool updateDebugMode(TMario *player) {
    player->mSpeed.set(0.0f, 0.0f, 0.0f);
    player->mForwardSpeed = 0.0f;

    gIsDebugActive = true;

    const bool shouldToggleDebug =
        ButtonsFramePressed(player->mController, gControlToggleDebugActive);

    const bool shouldToggleDebugState =
        ButtonsFramePressed(player->mController, gControlToggleDebugState) &&
        ButtonsPressed(player->mController, gSecondaryMask);

    const bool shouldIncreaseAnimationID =
        ButtonsFramePressed(player->mController, gControlIncreaseAnimationID) &&
        !ButtonsPressed(player->mController, gSecondaryMask);

    const bool shouldDecreaseAnimationID =
        ButtonsFramePressed(player->mController, gControlDecreaseAnimationID) &&
        !ButtonsPressed(player->mController, gSecondaryMask);

    const bool shouldIncreaseAnimationSpeed =
        ButtonsRapidPressed(player->mController, gControlIncreaseAnimationSpeed) &&
        ButtonsPressed(player->mController, gSecondaryMask);

    const bool shouldDecreaseAnimationSpeed =
        ButtonsRapidPressed(player->mController, gControlDecreaseAnimationSpeed) &&
        ButtonsPressed(player->mController, gSecondaryMask);

    const bool shoudToggleFastMovement =
        ButtonsFramePressedEx(player->mController, gControlToggleFastMovement);

    if (shouldToggleDebugState) {
        gDebugState += 1;
        if (gDebugState > 2)
            gDebugState = NONE;

        if (!sJustStartedDebug && gDebugState == NONE) {
            player->mState   = TMario::STATE_FALL;
            sJustExitedDebug = true;
            return true;
        } else if (gDebugState == CAM_MODE) {
            if (!gIsCameraFrozen || gIsCameraTracking) {
                gCamPosition = gpCamera->mTranslation;
                TVec3f translation, scale;
                Matrix::decompose(gpCamera->mTRSMatrix, translation, gCamRotation, scale);
            }
        }
    } else {
        sJustStartedDebug = false;
    }

    if (shoudToggleFastMovement) {
        gIsFastMovement ^= true;
    }

    switch (gDebugState) {
    default:
    case XYZ_MODE:
        DebugMoveMarioXYZ(player);
        break;
    case CAM_MODE:
        DebugCamera(player, gpCamera);
        break;
    }

    int prevAnimation = gAnimationID;
    while (!Player::isAnimationValid(gAnimationID) || gAnimationID == prevAnimation) {
        if (shouldIncreaseAnimationID)
            gAnimationID += 1;
        else if (shouldDecreaseAnimationID)
            gAnimationID -= 1;
        else
            break;

        if (gAnimationID < 0) {
            gAnimationID = sNumAnimations - 1;
            break;
        }
        if (gAnimationID >= sNumAnimations) {
            gAnimationID = 0;
            break;
        }
    }

    if (shouldIncreaseAnimationSpeed)
        gAnimationSpeed += 0.01;
    else if (shouldDecreaseAnimationSpeed)
        gAnimationSpeed -= 0.01;

    gAnimationSpeed = clamp(gAnimationSpeed, 0.0f, 1000.0f);

    if (Player::isAnimationValid(gAnimationID))
        player->setAnimation(gAnimationID, gAnimationSpeed);

    return false;
}

enum DebugNozzleKind { SPRAY, ROCKET, UNDERWATER, YOSHI, HOVER, TURBO };

static s32 sNozzleKind = DebugNozzleKind::SPRAY;

// extern -> debug update callback
BETTER_SMS_FOR_CALLBACK void updateFluddNozzle(TApplication *app) {
    TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

    if (!director || !BetterSMS::isDebugMode() ||
        director->mCurState != TMarDirector::Status::STATE_NORMAL)
        return;

    if (gDebugState != NONE)
        return;

    TMario *player   = gpMarioAddress;
    TWaterGun *fludd = player->mFludd;

    if (!fludd)
        return;

    const bool shouldSwitchNozzleR =
        ButtonsRapidPressed(player->mController, gControlNozzleSwitchR) &&
        !ButtonsPressed(player->mController, gSecondaryMask);

    const bool shouldSwitchNozzleL =
        ButtonsRapidPressed(player->mController, gControlNozzleSwitchL) &&
        !ButtonsPressed(player->mController, gSecondaryMask);

    sNozzleKind = fludd->mSecondNozzle;

    s32 adjust = 0;
    if (shouldSwitchNozzleR)
        adjust = 1;
    else if (shouldSwitchNozzleL)
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
        fludd->mCurrentNozzle      = fludd->mSecondNozzle;
        TNozzleBase *currentNozzle = fludd->mNozzleList[fludd->mCurrentNozzle];
        fludd->mCurrentWater       = currentNozzle->mEmitParams.mAmountMax.get();
    }

    sNozzleKind %= 7;
}

static bool shouldNotUpdateMarDirector() {
    TMarDirector *director;
    SMS_FROM_GPR(31, director);

    return director->mAreaID == 15 || (gDebugState != NONE);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80297A48, 0, 0, 0), shouldNotUpdateMarDirector);
SMS_WRITE_32(SMS_PORT_REGION(0x80297A4C, 0, 0, 0), 0x28030001);

static SMS_ASM_FUNC void flagConditionalGameUI() {
    SMS_ASM_BLOCK("lis 12, gIsGameUIActive@ha      \n\t"
                  "lbz 12, gIsGameUIActive@l (12)  \n\t"
                  "cmpwi 12, 1                     \n\t"
                  "beq _render_ui                  \n\t"
                  "li 4, 0                         \n\t"
                  "_render_ui:                     \n\t"
                  "rlwinm. 0, 4, 0, 31, 31         \n\t"
                  "blr                             \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80140844, 0, 0, 0), flagConditionalGameUI);

static u32 checkShouldUpdateMeaning() {
    TMarioGamePad *controller;
    SMS_FROM_GPR(30, controller);

    if (gIsDebugActive && gpApplication.mContext == TApplication::CONTEXT_DIRECT_STAGE)
        controller->mMeaning = 0;
    return controller->mMeaning;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802a8948, 0, 0, 0), checkShouldUpdateMeaning);
SMS_WRITE_32(SMS_PORT_REGION(0x802a894c, 0, 0, 0), 0x7c63f878);
SMS_WRITE_32(SMS_PORT_REGION(0x802a8950, 0, 0, 0), 0x907e00d4);

// static void setEmitPrm() {
//     TWaterBalloon::sEmitInfo = new TWaterEmitInfo("/mario/waterballoon/waterballoon.prm");
//     TParams::finalize();
// }
// SMS_PATCH_BL(SMS_PORT_REGION(0x802B8DC8, 0x802B0D98, 0, 0), setEmitPrm);