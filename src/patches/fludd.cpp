#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <JSystem/J2D/J2DPrint.hxx>
#include <JSystem/JKernel/JKRFileLoader.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/MapObj/MapObjNormalLift.hxx>
#include <SMS/MapObj/MapObjTree.hxx>

#include <SMS/Strategic/HitActor.hxx>
#include <SMS/Strategic/LiveActor.hxx>
#include <SMS/macros.h>
#include <SMS/System/CardManager.hxx>
#include <SMS/raw_fn.hxx>


#include "geometry.hxx"
#include "libs/constmath.hxx"
#include "module.hxx"
#include "p_settings.hxx"

#if BETTER_SMS_BUGFIXES

using namespace BetterSMS;
using namespace BetterSMS::Geometry;

// TODO: Make check for BetterSMS::areBugsPatched()
static SMS_ASM_FUNC bool makeWaterHitCheckForDeath(TBGCheckData *col) {
    // clang-format off
  SMS_ASM_BLOCK (
    "lhz 0, 0 (3)             \n\t"
    "cmpwi 0, 2048            \n\t"
    "bne .makeWaterCheck_tmp0 \n\t"
    "li 0, 1025               \n\t"
    ".makeWaterCheck_tmp0:    \n\t"
    SMS_PORT_REGION (
      "lis 12, 0x8018           \n\t"
      "ori 12, 12, 0xC36C       \n\t",

      "lis 12, 0x8018           \n\t"
      "ori 12, 12, 0x4bf4       \n\t",

      "lis 12, 0           \n\t"
      "ori 12, 12, 0       \n\t",

      "lis 12, 0           \n\t"
      "ori 12, 12, 0       \n\t"
    )
    "mtctr 12                 \n\t"
    "bctr                     \n\t"
  );
    // clang-format on
}
SMS_PATCH_B(SMS_PORT_REGION(0x8018C368, 0x80184BF0, 0, 0), makeWaterHitCheckForDeath);

static void normalizeHoverSlopeSpeed(f32 floorPos) {
    TMario *player;
    SMS_FROM_GPR(22, player);

    player->mPosition.y = floorPos;

    if (!BetterSMS::areBugsPatched())
        return;

    if (!(player->mState == static_cast<u32>(TMario::STATE_HOVER)))
        return;

    const f32 playerRotY    = f32(player->mAngle.y) / 182.0f;
    const Vec playerForward = {sinf(angleToRadians(-playerRotY)), 0.0f,
                               cosf(angleToRadians(playerRotY))};
    const Vec up            = {0.0f, 1.0f, 0.0f};

    Vec floorNormal;
    PSVECNormalize(reinterpret_cast<Vec *>(player->mFloorTriangle->getNormal()), &floorNormal);

    const f32 slopeStrength = PSVECDotProduct(&up, &floorNormal);
    if (slopeStrength > 0.7f)
        return;

    const f32 lookAtRatio = Vector3::lookAtRatio(playerForward, floorNormal);
    if (isnan(lookAtRatio))
        return;

    player->mForwardSpeed =
        Min(player->mForwardSpeed,
            10.0f * clamp(scaleLinearAtAnchor(slopeStrength, lookAtRatio, 1.0f), 0.0f, 1.0f));
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802568F0, 0x8024E67C, 0, 0), normalizeHoverSlopeSpeed);

#endif

static f32 checkTurboSpecial() {
    TNozzleTrigger *nozzle;
    SMS_FROM_GPR(29, nozzle);

    if (nozzle->mFludd->mCurrentNozzle == TWaterGun::Turbo) {
        return 0.01f;
    }

    return nozzle->mEmitParams.mInsidePressureMax.get();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8026C5BC, 0, 0, 0), checkTurboSpecial);

static void checkFillMax() {
    TNozzleTrigger *nozzle;
    SMS_FROM_GPR(29, nozzle);

    if (nozzle->mFludd->mCurrentNozzle == TWaterGun::Turbo)
        return;

    nozzle->mTriggerFill = nozzle->mEmitParams.mInsidePressureMax.get();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8026C5C8, 0, 0, 0), checkFillMax);

static void turboNozzleConeCondition() { /* TMarioEffect * */
    u32 *marioEffect;
    SMS_FROM_GPR(29, marioEffect);

    u32 state = 1;

    TMario *player = reinterpret_cast<TMario *>(marioEffect[0x68 / 4]);
    if (player->mFludd->mCurrentNozzle == TWaterGun::Spray)
        state = 0;

    marioEffect[0x7C / 4] = state;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80271ACC, 0, 0, 0), turboNozzleConeCondition);

// We return TMario to exploit registers
static TMario *turboNozzleConeCondition2(TMario *player) { /* TMarioEffect * */
    u32 *marioEffect;
    SMS_FROM_GPR(29, marioEffect);

    if (!player->mAttributes.mIsFluddEmitting)
        return nullptr;

    MActor *coneActor = reinterpret_cast<MActor *>(marioEffect[0x80 / 4]);
    if (player->mFludd->mCurrentNozzle == TWaterGun::Turbo && player->mController) {
        coneActor->mModel->mBaseScale.set(player->mController->mButtons.mAnalogR,
                                          player->mController->mButtons.mAnalogR,
                                          player->mController->mButtons.mAnalogR);
    } else {
        coneActor->mModel->mBaseScale.set(1.0f, 1.0f, 1.0f);
    }

    return player;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80271AD8, 0, 0, 0), turboNozzleConeCondition2);
SMS_WRITE_32(SMS_PORT_REGION(0x80271ADC, 0, 0, 0), 0x2C030000);

static void lerpTurboNozzleSpeed(TMario *player, f32 velocity) {
    auto *controller = player->mController;
    if (!controller) {
        player->setPlayerVelocity(velocity);
        return;
    }

    auto analogR = controller->mButtons.mAnalogR;
    velocity     = lerp<f32>(40.0f, velocity, analogR);
    player->setPlayerVelocity(velocity);

    player->mRunParams.mDashRotSp.set(lerp<s16>(180, 60, analogR));
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8025B2B0, 0, 0, 0), lerpTurboNozzleSpeed);
SMS_PATCH_BL(SMS_PORT_REGION(0x80272D40, 0, 0, 0), lerpTurboNozzleSpeed);

static void lerpTurboNozzleJumpSpeed() {
    TMario *player;
    SMS_FROM_GPR(30, player);

    auto *controller = player->mController;
    if (!controller) {
        player->mForwardSpeed = player->mJumpParams.mBroadJumpForce.get();
        player->mSpeed.y      = player->mJumpParams.mBroadJumpForceY.get();
        return;
    }

    auto analogR          = controller->mButtons.mAnalogR;
    player->mForwardSpeed = lerp<f32>(30.0f, player->mJumpParams.mBroadJumpForce.get(), analogR);
    player->mSpeed.y      = lerp<f32>(player->mJumpParams.mBroadJumpForceY.get() * 0.5f,
                                 player->mJumpParams.mBroadJumpForceY.get(), analogR);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80254990, 0, 0, 0), lerpTurboNozzleJumpSpeed);

#define SCALE_PARAM(param, scale) param.set(param.get() * scale)

void initTurboMaxCapacity(TMario *player, bool isMario) {
    if (!isMario)
        return;

    if (!player->mFludd)
        return;

    SCALE_PARAM(player->mFludd->mNozzleTurbo.mEmitParams.mAmountMax, 8);
    SCALE_PARAM(player->mFludd->mNozzleTurbo.mEmitParams.mDamageLoss, 8);
    SCALE_PARAM(player->mFludd->mNozzleTurbo.mEmitParams.mSuckRate, 8);
    player->mDeParams.mDashStartTime.set(0.0f);
}

void updateTurboFrameEmit(TMario *player, bool isMario) {
    if (!isMario)
        return;

    auto *fludd = player->mFludd;
    if (!fludd || !player->mController)
        return;

    if (fludd->mCurrentNozzle != TWaterGun::Turbo)
        return;

    const auto analogR = player->mController->mButtons.mAnalogR;

    fludd->mNozzleTurbo.mEmitParams.mNum.set(lerp<f32>(1.0f, 10.0f, analogR));
    fludd->mNozzleTurbo.mEmitParams.mDirTremble.set(lerp<f32>(0.01f, 0.08f, analogR));

    if (analogR < 0.1f || fludd->mNozzleTurbo.mSprayState == 2) {
        fludd->mNozzleTurbo.mTriggerFill = 0.0f;
        return;
    }

    fludd->mNozzleTurbo.mTriggerFill = fludd->getPressureMax() * ((analogR - 0.15f) * 1.17647f);
    player->mDeParams.mDashAcc.set(32.1f);  // 32.0f is max
}

#undef SCALE_PARAM