#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <SMS/enemy/EnemyMario.hxx>
#include <SMS/SMS.hxx>
#include <SMS/actor/Mario.hxx>
#include <SMS/npc/BaseNPC.hxx>
#include <SMS/sound/MSound.hxx>
#include <SMS/sound/MSoundSESystem.hxx>

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
void drawCheatText(TMarDirector *director, J2DOrthoGraph *graph) {
    if (!gDebugTextBoxW->getStringPtr())
        return;

    if (BetterSMS::isDebugMode()) {
        gDebugTextBoxB->draw(235, 462);
        gDebugTextBoxW->draw(234, 460);
    }
}

static void *handleDebugCheat(void *GCLogoDir) {
    if (!gDebugHandler.isInitialized()) {
        gDebugHandler.setGamePad(gpApplication.mGamePad1);
        gDebugHandler.setInputList(gDebugModeCheatCode);
        gDebugHandler.setSuccessCallBack(&debugModeNotify);

        auto *currentHeap               = JKRHeap::sSystemHeap->becomeCurrentHeap();
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

bool gInXYZMode = false;

void initMarioXYZMode(TMarDirector *director) { gInXYZMode = false; }

// extern -> debug update callback
// 0x8024D194
void updateMarioXYZMode(TMarDirector *director) {
    constexpr f32 baseSpeed = 83.0f;
    constexpr u32 buttons   = TMarioGamePad::EButtons::DPAD_UP;

    if (!director || !BetterSMS::isDebugMode() ||
        director->mCurState != TMarDirector::Status::NORMAL)
        return;

    const JUTGamePad::CStick &mainStick = gpMarioAddress->mController->mControlStick;
    const f32 speedMultiplier = lerp<f32>(1, 2, gpMarioAddress->mController->mButtons.mAnalogR);

    if (gpMarioAddress->mController->mButtons.mFrameInput & buttons && !gInXYZMode) {
        gInXYZMode = true;
    } else if (gpMarioAddress->mController->mButtons.mFrameInput & buttons && gInXYZMode) {
        gInXYZMode = false;
    }

    if (gInXYZMode) {
        const f32 cameraRotY = (f32)(gpCamera->mHorizontalAngle) / 182.0f;

        Vec playerPos;
        gpMarioAddress->JSGGetTranslation(&playerPos);

        playerPos.x +=
            ((-sinf(angleToRadians(cameraRotY)) * baseSpeed) * speedMultiplier) * mainStick.mStickY;
        playerPos.z +=
            ((-cosf(angleToRadians(cameraRotY)) * baseSpeed) * speedMultiplier) * mainStick.mStickY;
        playerPos.x -= ((-sinf(angleToRadians(cameraRotY + 90.0f)) * baseSpeed) * speedMultiplier) *
                       mainStick.mStickX;
        playerPos.z -= ((-cosf(angleToRadians(cameraRotY + 90.0f)) * baseSpeed) * speedMultiplier) *
                       mainStick.mStickX;

        if (gpMarioAddress->mController->mButtons.mInput & TMarioGamePad::EButtons::B) {
            playerPos.y -= (baseSpeed * speedMultiplier);
        } else if (gpMarioAddress->mController->mButtons.mInput & TMarioGamePad::EButtons::A) {
            playerPos.y += (baseSpeed * speedMultiplier);
        }

        gpMarioAddress->JSGSetTranslation(playerPos);
    }
    return;
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

//         sHomeTriangle->mCollisionType          = 16042;
//         sHomeTriangle->mValue4                 = s16((sTargetID << 8) | (sHomeID - 1));
//         player->mFloorTriangle->mCollisionType = 16042;
//         player->mFloorTriangle->mValue4        = s16(((sHomeID - 1) << 8) | sTargetID);
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
    extern bool gInXYZMode;
    return gInXYZMode ? 0x224E0 : player->mState;  // Spoof non dying value
};
SMS_PATCH_BL(SMS_PORT_REGION(0x80243110, 0x8023AE9C, 0, 0), preventDebuggingDeath);

static void preventDebuggingInteraction(TMario *player, JDrama::TGraphics *graphics) {
    extern bool gInXYZMode;
    if (!gInXYZMode)
        player->playerControl(graphics);
    else {
        player->mForwardSpeed = 0.0f;
        player->mSpeed        = {0.0f, 0.0f, 0.0f};
        player->mState        = static_cast<u32>(TMario::STATE_IDLE);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024D3A0, 0x8024512C, 0, 0), preventDebuggingInteraction);