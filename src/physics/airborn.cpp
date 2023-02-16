#include <Dolphin/MTX.h>
#include <Dolphin/math.h>
#include <Dolphin/types.h>

#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Player/Mario.hxx>

#include "libs/constmath.hxx"
#include "libs/triangle.hxx"
#include "module.hxx"
#include "player.hxx"

#include "p_geometry.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Geometry;

static void doProcessJumpState(TMario *player) {
    auto playerData = Player::getData(player);

    TMarioGamePad *controller = player->mController;

    const f32 stickMagnitude = controller->mControlStick.mLengthFromNeutral;

#if 1
    if (player->mState != static_cast<u32>(TMario::STATE_JUMPSPINR) &&
        player->mState != static_cast<u32>(TMario::STATE_JUMPSPINL))
        playerData->mCollisionFlags.mIsSpinBounce = false;

    if (playerData->mCollisionFlags.mIsSpinBounce) {
        if (stickMagnitude > 0.1f) {
            Player::rotateRelativeToCamera(
                player, gpCamera,
                {controller->mControlStick.mStickX, controller->mControlStick.mStickY}, 1.0f);
        } else {
            player->mForwardSpeed *= 0.98f;
        }
    }
#else
    Player::rotateRelativeToCamera(
        player, gpCamera, {controller->mControlStick.mStickX, controller->mControlStick.mStickY},
        1.0f);
#endif

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

static void checkJumpSpeedLimit(f32 speed) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    auto playerData = Player::getData(player);

    f32 speedCap     = 32.0f;
    f32 speedReducer = 0.2f;

    if (!player->onYoshi()) {
        speedCap *= playerData->getParams()->mSpeedMultiplier.get();
        speedReducer *=
            scaleLinearAtAnchor<f32>(playerData->getParams()->mSpeedMultiplier.get(), 3.0f, 1.0f);
    }

    if (speed > speedCap) {
        player->mForwardSpeed = (speed - speedReducer);
    }
}
SMS_WRITE_32(SMS_PORT_REGION(0x8024CB00, 0x8024488C, 0, 0),
             SMS_PORT_REGION(0xC162EF70, 0xC162EDE8, 0, 0));
SMS_WRITE_32(SMS_PORT_REGION(0x8024CC50, 0x802449DC, 0, 0), 0xED600072);
SMS_WRITE_32(SMS_PORT_REGION(0x8024CC60, 0x802449EC, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8024CC64, 0x802449F0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8024CC68, 0x802449F4, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x8024CC6C, 0x802449F8, 0, 0), checkJumpSpeedLimit);

static TMario *checkJumpSpeedMulti(TMario *player, f32 factor, f32 max) {
    auto playerData = Player::getData(player);

    if (playerData->isMario() && !player->onYoshi()) {
        player->mForwardSpeed = ((factor * playerData->getParams()->mSpeedMultiplier.get()) * max) +
                                player->mForwardSpeed;
        return player;
    } else {
        player->mForwardSpeed = (factor * max) + player->mForwardSpeed;
        return player;
    }
}
SMS_WRITE_32(SMS_PORT_REGION(0x8024CCF8, 0x80244A84, 0, 0), 0xEC0B007A);
SMS_WRITE_32(SMS_PORT_REGION(0x8024CD24, 0x80244AB0, 0, 0), 0xEC0B007A);
SMS_PATCH_BL(SMS_PORT_REGION(0x8024CC2C, 0x802449B8, 0, 0), checkJumpSpeedMulti);
SMS_WRITE_32(SMS_PORT_REGION(0x8024CC30, 0x802449BC, 0, 0), 0x57C5043E);

#if BETTER_SMS_DYNAMIC_FALL_DAMAGE

static f32 getTrueFloorContactSpeed(TMario *player) {
    const f32 playerRotY    = f32(player->mAngle.y) / 182.0f;
    const Vec playerForward = {sinf(angleToRadians(-playerRotY)), 0.0f,
                               cosf(angleToRadians(playerRotY))};
    const Vec up            = {0.0f, 1.0f, 0.0f};

    Vec floorNormal;
    PSVECNormalize(reinterpret_cast<Vec *>(player->mFloorTriangle->getNormal()), &floorNormal);

    const f32 slopeStrength = PSVECDotProduct(&up, &floorNormal);
    const f32 lookAtRatio   = Vector3::lookAtRatio(playerForward, floorNormal);
    if (isnan(lookAtRatio))
        return player->mSpeed.y;

    return (player->mSpeed.y - (((lookAtRatio - 0.5f) * 2.0f) * player->mForwardSpeed)) *
           slopeStrength;
}

static void dynamicFallDamage(TMario *player, int dmg, int type, int emitcount, int tremble) {
    auto playerData = Player::getData(player);

    dmg *= static_cast<int>((player->mLastGroundedPos.y - player->mTranslation.y) /
                            (player->mDeParams.mDamageFallHeight.get() / 1.5f));

    const f32 terminalVelocity = -75.0f * player->mJumpParams.mGravity.get();
    const f32 trueContact      = getTrueFloorContactSpeed(player);
    dmg                        = Max(static_cast<int>(dmg * (trueContact / terminalVelocity)), 1);

    if (dmg > 2) {
        type      = 0;  // shaky
        emitcount = (dmg - 2) * 3;
    }

    if (player->mSpeed.y <= (-75.0f * player->mJumpParams.mGravity.get()) + 5.0f)
        player->floorDamageExec(dmg, type, emitcount, tremble);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024C73C, 0x80246BB4, 0, 0), dynamicFallDamage);

#endif

// 8024afe0 <- hover air Y spd

static SMS_ASM_FUNC void scaleFluddInitYSpd() {
    SMS_ASM_BLOCK("lfs 0, " SMS_STRINGIZE(
        SMS_PORT_REGION(-0xFE0, -0x1168, 0, 0)) "(2)  \n\t"
                                                "lfs 4, 0x28(30)                          \n\t"
                                                "fmuls 0, 0, 4                            \n\t"
                                                "blr                                      \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80254A2C, 0x8024C7B8, 0, 0), scaleFluddInitYSpd);