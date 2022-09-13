#include <Dolphin/types.h>
#include <SMS/SMS.hxx>
#include <SMS/actor/Mario.hxx>
#include <SMS/raw_fn.hxx>

#include "module.hxx"
#include "player.hxx"

#if BETTER_SMS_MULTI_JUMP

using namespace BetterSMS;

extern f32 calcJumpPower(TMario *player, f32 factor, f32 base, f32 jumpPower);

// extern -> SME.cpp
// 0x8024E02C
static void manageCustomJumps(TMario *player) {
    auto *playerData = Player::getData(player);
    if (!playerData) {
        stateMachine__6TMarioFv(player);
        return;
    }

    const s32 jumpsLeft = (playerData->getParams()->mMaxJumps.get() - playerData->mCurJump);

    if ((player->mState & static_cast<u32>(TMario::STATE_AIRBORN)) == false ||
        (player->mState & 0x800000) || player->mYoshi->mState == TYoshi::MOUNTED ||
        player->mState == static_cast<u32>(TMario::STATE_SLIP_JUMP) ||
        player->mState == static_cast<u32>(TMario::STATE_THROWN) ||
        player->mAttributes.mIsGameOver) {
        playerData->mCurJump = 1;
    } else if ((player->mController->mButtons.mFrameInput & TMarioGamePad::EButtons::A) &&
               jumpsLeft > 0 && player->mState != static_cast<u32>(TMario::STATE_WALLSLIDE) &&
               player->mState != static_cast<u32>(TMario::STATE_F_KNCK_H)) {
        u32 state   = player->mState;
        u32 voiceID = 0;
        u32 animID  = 0;

        if (jumpsLeft == 1) {
            state   = static_cast<u32>(TMario::STATE_TRIPLE_J);
            voiceID = 0x78B6;
            animID  = 0x6F;
        } else if (jumpsLeft % 2) {
            state   = static_cast<u32>(TMario::STATE_JUMP);
            voiceID = 0x78B1;
            animID  = 0x4D;
        } else {
            state   = static_cast<u32>(TMario::STATE_D_JUMP);
            voiceID = 0x78B6;
            animID  = 0x50;
        }

        startVoice__6TMarioFUl(player, voiceID);
        setAnimation__6TMarioFif(player, animID, 1.0f);

        TMarioGamePad *controller = player->mController;
        const f32 stickMagnitude  = controller->mControlStick.mLengthFromNeutral;

        if (stickMagnitude > 0.1f) {
            Player::rotateRelativeToCamera(
                player, gpCamera,
                {controller->mControlStick.mStickX, controller->mControlStick.mStickY}, 1.0f);
        }

        player->mForwardSpeed *= stickMagnitude;
        player->mSpeed.y   = calcJumpPower(player, 0.25f, player->mSpeed.y, 65.0f);
        player->mPrevState = player->mState;
        player->mState     = state;

        playerData->mIsLongJumping = false;
        playerData->mCurJump += 1;
    }
    stateMachine__6TMarioFv(player);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024E02C, 0x80245DB8, 0, 0), manageCustomJumps);

#endif