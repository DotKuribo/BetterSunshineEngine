#include <Dolphin/types.h>
#include <SMS/Player/Mario.hxx>

#include <SMS/raw_fn.hxx>

#include "module.hxx"
#include "player.hxx"

#if BETTER_SMS_MULTI_JUMP || 1

using namespace BetterSMS;

u32 MultiJumpState = 0xF00001C1;

void checkForMultiJump(TMario *player, bool isMario) {
    auto *playerData = Player::getData(player);
    if (!playerData)
        return;

    const bool isYoshi   = player->mYoshi ? player->mYoshi->mState == TYoshi::MOUNTED : false;
    const bool isAirBorn = (player->mState & TMario::STATE_AIRBORN) && (player->mActionState & 0x4);
    const bool isInvalidState =
        (player->mState & 0x800000) || isYoshi || player->mState == TMario::STATE_SLIP_JUMP ||
        player->mState == TMario::STATE_THROWN || player->mAttributes.mIsGameOver;

    if (!isAirBorn || isInvalidState) {
        playerData->mCurJump = 1;
        return;
    }

    if (player->mFludd && player->mFludd->mIsEmitWater &&
        player->mFludd->mCurrentNozzle == TWaterGun::Hover)
        return;

    const s32 jumpsLeft = (playerData->getParams()->mMaxJumps.get() - playerData->mCurJump);

    if ((player->mController->mButtons.mFrameInput & TMarioGamePad::EButtons::A) && jumpsLeft > 0 &&
        player->mState != TMario::STATE_WALLSLIDE && player->mState != TMario::STATE_F_KNCK_H) {
        player->changePlayerStatus(MultiJumpState, 0, false);
    }
}

bool processMultiJump(TMario *player) {
    auto *playerData = Player::getData(player);
    
    const s32 jumpsLeft = (playerData->getParams()->mMaxJumps.get() - playerData->mCurJump);

    u32 state   = player->mState;
    int voiceID = 0;
    int animID  = 0;

    if (jumpsLeft == 1) {
        state   = static_cast<u32>(TMario::STATE_D_JUMP);
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

    player->startVoice(voiceID);
    player->setAnimation(animID, 1.0f);

    const TMarioGamePad *controller = player->mController;
    const f32 stickMagnitude        = controller->mControlStick.mLengthFromNeutral;

    if (stickMagnitude > 0.1f) {
        Player::rotateRelativeToCamera(
            player, gpCamera,
            {controller->mControlStick.mStickX, controller->mControlStick.mStickY}, 1.0f);
    }

    playerData->mIsLongJumping = false;
    playerData->mCurJump += 1;

    player->mForwardSpeed *= stickMagnitude;
    player->changePlayerJumping(state, 0);

    return true;
}

static void playDoubleOrTripleAnim(TMario *player, int state, int anim, int unk_0) {
    auto *playerData = Player::getData(player);
    if (playerData->getParams()->mMaxJumps.get() - playerData->mCurJump == 0 &&
        player->mState == MultiJumpState) {
        player->jumpingBasic(state, 0x6F, unk_0);
        player->startVoice(0x78B6);
    }
    else
        player->jumpingBasic(state, anim, unk_0);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80249618, 0, 0, 0), playDoubleOrTripleAnim);

#endif