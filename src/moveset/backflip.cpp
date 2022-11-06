#include <SMS/Player/Mario.hxx>

#include <SMS/System/MarDirector.hxx>
#include <SMS/Player/Watergun.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>

#include "module.hxx"
#include "player.hxx"

using namespace BetterSMS;

u32 CrouchState = 0xF00001C0;

#if BETTER_SMS_SWAP_LZ_BUTTONS

SMS_WRITE_32(SMS_PORT_REGION(0x80249670, 0, 0, 0), 0x540004E7);
SMS_WRITE_32(SMS_PORT_REGION(0x80249730, 0, 0, 0), 0x540004E7);
SMS_WRITE_32(SMS_PORT_REGION(0x80249C34, 0, 0, 0), 0x540004E7);
SMS_WRITE_32(SMS_PORT_REGION(0x8024BF30, 0, 0, 0), 0x540004E7);
SMS_WRITE_32(SMS_PORT_REGION(0x8024C248, 0, 0, 0), 0x540004E7);
SMS_WRITE_32(SMS_PORT_REGION(0x8024C36C, 0, 0, 0), 0x54000427);
SMS_WRITE_32(SMS_PORT_REGION(0x80252124, 0, 0, 0), 0x540004E7);
SMS_WRITE_32(SMS_PORT_REGION(0x802A8834, 0, 0, 0), 0x54000673);
SMS_WRITE_32(SMS_PORT_REGION(0x802A8840, 0, 0, 0), 0x54000673);
SMS_WRITE_32(SMS_PORT_REGION(0x802A8860, 0, 0, 0), 0x540006F7);
SMS_WRITE_32(SMS_PORT_REGION(0x802A886C, 0, 0, 0), 0x540006F7);
SMS_WRITE_32(SMS_PORT_REGION(0x802A88C8, 0, 0, 0), 0x540006F7);
SMS_WRITE_32(SMS_PORT_REGION(0x802A88D4, 0, 0, 0), 0x540006F7);

constexpr u32 TargetMeaning = 0x1000;  // L

#else

constexpr u32 TargetMeaning = 0x100;  // Z

#endif

#if BETTER_SMS_BACK_FLIP || 1

SMS_WRITE_32(SMS_PORT_REGION(0x802A884C, 0, 0, 0), 0x60000000);  // Allow L button meaning updates
SMS_WRITE_32(SMS_PORT_REGION(0x8024E5CC, 0, 0, 0), 0x60000000);  // Allow nozzle change on backflip

void checkForCrouch(TMario *player, bool isMario) {
    if (gpMarDirector->mCurState != TMarDirector::Status::STATE_NORMAL)
        return;

    if (player->onYoshi())
        return;

    player->checkEnforceJump();

    if (player->mState != TMario::STATE_IDLE && player->mState != TMario::STATE_STOP &&
        player->mState != TMario::STATE_JMP_LAND && player->mState != TMario::STATE_HVY_LAND &&
        player->mState != TMario::STATE_LAND_RECOVER && player->mState != TMario::STATE_D_LAND &&
        player->mState != TMario::STATE_D_LAND_RECOVER)
        return;

    if ((player->mActionState & 0x8) || player->isForceSlip())
        return;

    if (player->mForwardSpeed > __FLT_EPSILON__)
        return;

    auto *controller = player->mController;
    if (!controller)
        return;

    if (!(controller->mMeaning & TargetMeaning))
        return;

    player->changePlayerStatus(CrouchState, 0, false);
    player->mSpeed.scale(0.2f);
}

bool processCrouch(TMario *player) {
    if (player->mPosition.y - player->mFloorBelow > 10.0f) {
        player->changePlayerStatus(TMario::STATE_FALL, 0, false);
        return true;
    }

    if (player->mActionState & 0x8 || player->isForceSlip()) {
        player->changePlayerStatus(0x50, 0, false);
        return true;
    }

    auto *fludd = player->mFludd;
    if (fludd) {
        if (fludd->mIsEmitWater && fludd->mCurrentNozzle != TWaterGun::Spray) {
            player->changePlayerStatus(TMario::STATE_IDLE, 0, false);
            return true;
        }
    }

    auto *controller = player->mController;
    if (!controller) {
        player->changePlayerStatus(TMario::STATE_SIDE_STEP_LEAVE, 0, false);
        return true;
    }

    if (!(controller->mMeaning & TargetMeaning)) {
        player->changePlayerStatus(TMario::STATE_SIDE_STEP_LEAVE, 0, false);
        return true;
    }

    auto &controlStick = controller->mControlStick;

    const bool backHeld = controlStick.mAngle < 0x4000 && controlStick.mAngle > -0x4000;
    if (controller->mControlStick.mLengthFromNeutral > 0.8f && !backHeld) {
        player->changePlayerStatus(TMario::STATE_RUNNING, 0, false);
        return true;
    }

    if (controller->mButtons.mFrameInput & TMarioGamePad::A) {
        player->changePlayerStatus(TMario::STATE_BACK_FLIP, 0, false);
        return true;
    }

    // Check walls
    float normalThing = player->mFloorTriangle ? player->mFloorTriangle->mNormal.y : 0.0f;
    TVec3f succ{player->mPosition.x + (player->mSpeed.x * normalThing * 0.25f), player->mPosition.y,
                player->mPosition.z + (player->mSpeed.z * normalThing * 0.25f)};
    player->checkGroundAtWalking(succ);
    player->checkCollision();

    player->mSpeed.y = 0;
    player->mSpeed.scale(0.95);
    player->mPosition.add(player->mSpeed);
    player->mForwardSpeed *= 0.95;

    player->setAnimation(TMario::ANIMATION_STEADY_STANCE, 1.0f);
    return false;
}

#endif