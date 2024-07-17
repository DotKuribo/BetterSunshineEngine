#include <Dolphin/MTX.h>
#include <Dolphin/math.h>
#include <Dolphin/types.h>

#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Player/Mario.hxx>

#include "libs/constmath.hxx"
#include "libs/triangle.hxx"
#include "module.hxx"
#include "player.hxx"

#include "libs/geometry.hxx"

using namespace BetterSMS;

static void doProcessJumpState(TMario *player) {
    auto playerData = Player::getData(player);

    TMarioGamePad *controller = player->mController;

    const f32 stickMagnitude = controller->mControlStick.mLengthFromNeutral;

    if (player->mState != static_cast<u32>(TMario::STATE_JUMPSPINR) &&
        player->mState != static_cast<u32>(TMario::STATE_JUMPSPINL))
        playerData->mCollisionFlags.mIsSpinBounce = false;

    if (playerData->mCollisionFlags.mIsSpinBounce) {
        if (stickMagnitude > 0.1f) {
            Player::rotateRelativeToCamera(
                player, gpCamera,
                {controller->mControlStick.mStickX, controller->mControlStick.mStickY}, 0.1f);
        } else {
            player->mForwardSpeed *= 0.98f;
        }
    }

    player->jumpMain();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80250138, 0x80247EC4, 0, 0), doProcessJumpState);

static void checkYSpdForTerminalVelocity() {
    TMario *player;
    SMS_FROM_GPR(31, player);

    auto playerData = Player::getData(player);

    float terminalVelocity;
    if (playerData->mCollisionFlags.mIsSpinBounce)
        terminalVelocity = -20.0f * player->mJumpParams.mGravity.get();
    else
        terminalVelocity = -75.0f * player->mJumpParams.mGravity.get();
    player->mSpeed.y = Max(player->mSpeed.y, terminalVelocity);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80256678, 0x8024E404, 0, 0), checkYSpdForTerminalVelocity);
SMS_WRITE_32(SMS_PORT_REGION(0x8025667C, 0x8024E408, 0, 0), 0x60000000);

// 8024afe0 <- hover air Y spd

static SMS_ASM_FUNC void scaleFluddInitYSpd() {
    SMS_ASM_BLOCK("lfs 0, " SMS_STRINGIZE(
        SMS_PORT_REGION(-0xFE0, -0x1168, 0, 0)) "(2)  \n\t"
                                                "lfs 4, 0x28(30)                          \n\t"
                                                "fmuls 0, 0, 4                            \n\t"
                                                "blr                                      \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80254A2C, 0x8024C7B8, 0, 0), scaleFluddInitYSpd);