#include <Dolphin/types.h>

#include <SMS/Player/Mario.hxx>

#include <SMS/raw_fn.hxx>

#include "libs/constmath.hxx"
#include "module.hxx"
#include "player.hxx"
#include "stage.hxx"

using namespace BetterSMS;

f32 calcJumpPower(TMario *player, f32 factor, f32 base, f32 jumpPower) {
    base = Min(base, 100.0f);

    auto *playerData = Player::getData(player);
    if (!playerData)
        return Max(base, (base * factor) + jumpPower);

    auto *params = playerData->getParams();
    if (!params)
        return Max(base, (base * factor) + jumpPower);

    auto *stageConfig = Stage::getStageConfiguration();

    jumpPower *= params->mBaseJumpMultiplier.get();
    jumpPower *=
        scaleLinearAtAnchor<f32>(params->mSizeMultiplier.get() *
                                     (stageConfig ? stageConfig->mPlayerSizeMultiplier.get() : 0),
                                 0.5f, 1.0f);
    if (player->mState & TMario::STATE_AIRBORN) {
        f32 multiplier = params->mMultiJumpMultiplier.get();
        for (u32 i = 1; i < playerData->mCurJump; ++i) {
            multiplier *= multiplier;
        }
        jumpPower *= multiplier;
        player->mForwardSpeed *= params->mMultiJumpFSpeedMulti.get();
    }

    return Max(base, (base * factor) + jumpPower);
}

#if BETTER_SMS_LONG_JUMP

static void setJumpOrLongJump(TMario *player, u32 state, u32 unk_0) {
    constexpr u32 LongJumpSpecifier = TMarioGamePad::EButtons::L;
    constexpr f32 LongJumpMinSpeed  = 10.0f;

    auto *playerData = Player::getData(player);
    if (!playerData) {
        player->setStatusToJumping(state, unk_0);
        return;
    }
    auto &buttons = player->mController->mButtons;

    const bool isValidState =
        !(player->mState & TMario::STATE_AIRBORN) &&
                              !(player->mState & TMario::STATE_WATERBORN) && player->mState != TMario::STATE_DIVESLIDE &&
                              !player->onYoshi();

    playerData->mIsLongJumping = false;
    if ((buttons.mInput & LongJumpSpecifier) == LongJumpSpecifier &&
        (buttons.mFrameInput & TMarioGamePad::EButtons::A) &&
        player->mForwardSpeed > LongJumpMinSpeed && isValidState) {
        playerData->mIsLongJumping = playerData->isMario() && (player->mActionState & 0x8) == 0;
        state                      = TMario::STATE_JUMP;
    }

    player->setStatusToJumping(state, unk_0);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802541BC, 0x8024BF48, 0, 0), setJumpOrLongJump);

static void processJumpOrLongJumpAnim(TMario *player, int state, int anim, int unk_0) {
    auto *playerData = Player::getData(player);
    if (playerData && playerData->mIsLongJumping)
        anim = 0xF6;

    player->jumpingBasic(state, anim, unk_0);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80249554, 0x802412E0, 0, 0), processJumpOrLongJumpAnim);

static void processJumpOrLongJump() {
    TMario *player;
    SMS_FROM_GPR(30, player);

    constexpr f32 LongJumpSpeedForward = 36.0f;
    constexpr f32 LongJumpSpeedUp      = 50.0f;

    auto *playerData = Player::getData(player);
    if (!playerData) {
        player->mSpeed.y = calcJumpPower(player, 0.25f, player->mForwardSpeed, 42.0f);
        return;
    }

    if (!playerData->mIsLongJumping) {
        player->mSpeed.y = calcJumpPower(player, 0.25f, player->mForwardSpeed, 42.0f);
        return;
    }

    player->startVoice(TMario::VOICE_TRIPLE_JUMP);

    player->mSpeed.y += LongJumpSpeedUp * playerData->getParams()->mBaseJumpMultiplier.get();
    player->mForwardSpeed += LongJumpSpeedForward * playerData->getParams()->mSpeedMultiplier.get();
    player->mPrevState = player->mState;
    player->mState     = static_cast<u32>(TMario::STATE_JUMP);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80254534, 0x8024c2c0, 0, 0), processJumpOrLongJump);
SMS_WRITE_32(SMS_PORT_REGION(0x80254538, 0x8024c2c4, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8025453C, 0x8024c2c8, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80254540, 0x8024c2cc, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80254544, 0x8024c2d0, 0, 0), 0x60000000);

static bool checkDivingWhenLongJumping(TMario *player) {
    const bool onYoshi = player->onYoshi();

    auto *playerData = Player::getData(player);
    if (!playerData)
        return onYoshi;

    return onYoshi || playerData->mIsLongJumping;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024C394, 0x80244120, 0, 0), checkDivingWhenLongJumping);

static bool checkRotatingWhenLongJumping(TMario *player, int *unk_0) {
    const bool rotated = player->checkStickRotate(unk_0);

    auto *playerData = Player::getData(player);
    if (!playerData)
        return rotated;

    return rotated && !playerData->mIsLongJumping;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024C3F8, 0x80244184, 0, 0), checkRotatingWhenLongJumping);

static bool checkQuickFallWhenLongJumping() {
    TMario *player;
    SMS_FROM_GPR(30, player);

    const bool slowFalling = ((player->mActionState & 0x80) != 0);

    auto *playerData = Player::getData(player);
    if (!playerData)
        return slowFalling;

    return slowFalling || playerData->mIsLongJumping || playerData->mCollisionFlags.mIsSpinBounce;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802565D4, 0x8024E360, 0, 0), checkQuickFallWhenLongJumping);
SMS_WRITE_32(SMS_PORT_REGION(0x802565D8, 0x8024E364, 0, 0), 0x2C030000);

static bool preserveRegisterCheckQuickFall() {
    TMario *player;
    SMS_FROM_GPR(31, player);

    return player->mState == TMario::STATE_JUMPSPINR || player->mState == TMario::STATE_JUMPSPINL;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80256618, 0x8024E3A4, 0, 0), preserveRegisterCheckQuickFall);
SMS_WRITE_32(SMS_PORT_REGION(0x8025661C, 0x8024E3A8, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x80256620, 0x8024E3AC, 0, 0), 0x41820024);

#endif