#include <SMS/Player/Mario.hxx>

#include <SMS/game/MarDirector.hxx>
#include <SMS/nozzle/Watergun.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/sound/MSound.hxx>
#include <SMS/sound/MSoundSESystem.hxx>

#include "module.hxx"
#include "player.hxx"

// Remove arbitrary dive speed
SMS_WRITE_32(SMS_PORT_REGION(0x802496AC, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8024976C, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8024999C, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80249C70, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8024C3A8, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8024C284, 0, 0, 0), 0x60000000);

static void uncapDiveSpeed(f32 diveSpeed) {
    TMario *player;
    SMS_FROM_GPR(30, player);

	player->mForwardSpeed = Max(48.0f,
        player->mForwardSpeed + ((player->mState & TMario::STATE_AIRBORN) ? 15.0f : 0));
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80254920, 0, 0, 0), uncapDiveSpeed);

static void uncapDiveRolloutSpeed() {
    TMario *player;
    SMS_FROM_GPR(30, player);

    player->mForwardSpeed = Max(player->mJumpParams.mRotBroadJumpForce.get(), player->mForwardSpeed);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802549B0, 0, 0, 0), uncapDiveRolloutSpeed);