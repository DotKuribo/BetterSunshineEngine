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
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/System/CardManager.hxx>
#include <SMS/raw_fn.hxx>


#include "libs/geometry.hxx"
#include "libs/constmath.hxx"
#include "module.hxx"
#include "player.hxx"
#include "p_settings.hxx"

#if BETTER_SMS_BUGFIXES

using namespace BetterSMS;
using namespace BetterSMS::Geometry;

// This patches delayed fludd usage
static void snapNozzleToReady() {
    TWaterGun *fludd;
    SMS_FROM_GPR(30, fludd);

    if (BetterSMS::areBugsPatched() && fludd->mCurrentNozzle == TWaterGun::TNozzleType::Hover) {
        ((float *)(fludd))[0x1CEC / 4] = 0.0f;
    } else {
        ((float *)(fludd))[0x1CEC / 4] -= 0.1f;
        if (((float *)(fludd))[0x1CEC / 4] < 0.0f)
            ((float *)(fludd))[0x1CEC / 4] = 0.0f;
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802699CC, 0, 0, 0), snapNozzleToReady);

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

static bool hasWaterCardOpen() {
    TGCConsole2 *gcConsole;
    SMS_FROM_GPR(31, gcConsole);

    if (gpMarioAddress->mYoshi->mState != TYoshi::State::MOUNTED &&
        !gpMarioAddress->mAttributes.mHasFludd && !gcConsole->mWaterCardFalling &&
        gcConsole->mIsWaterCard)
        startDisappearTank__11TGCConsole2Fv(gcConsole);
    else if (gpMarioAddress->mYoshi->mState == TYoshi::State::MOUNTED)
        gpMarioAddress->mAttributes.mHasFludd = true;

    return gcConsole->mIsWaterCard;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8014206C, 0x80136C80, 0, 0), hasWaterCardOpen);
SMS_WRITE_32(SMS_PORT_REGION(0x80142070, 0x80136C84, 0, 0), 0x28030000);

static bool canCollectFluddItem(TMario *player) {
    const bool defaultEnabled = player->onYoshi();

    auto *playerData = Player::getData(gpMarioAddress);
    if (!playerData)
        return defaultEnabled;

    return defaultEnabled || !playerData->getCanUseFludd();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80283058, 0x8027ADE4, 0, 0), canCollectFluddItem);

static s32 sNozzleBuzzCounter = -1;
static bool canCollectFluddItem_() {
    TMario *player;
    SMS_FROM_GPR(30, player);

    const bool isOnYoshi = player->onYoshi();

    auto *playerData = Player::getData(gpMarioAddress);
    if (!playerData)
        return isOnYoshi;

    if (!playerData->getCanUseFludd()) {
        if (gpMSound->gateCheck(0x483E) && sNozzleBuzzCounter < 0) {
            MSoundSESystem::MSoundSE::startSoundSystemSE(0x483E, 0, nullptr, 0);
            sNozzleBuzzCounter = 120;
        } else {
            sNozzleBuzzCounter -= 1;
        }
    }
    return isOnYoshi || !playerData->getCanUseFludd();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BBD48, 0x801B3C00, 0, 0), canCollectFluddItem_);

static void resetNozzleBuzzer(TMapObjGeneral *obj) {
    if (obj->mNumObjs <= 0) {
        sNozzleBuzzCounter = Max(sNozzleBuzzCounter - 1, -1);
    }
    control__14TMapObjGeneralFv(obj);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BBBF8, 0x801B3AB0, 0, 0), resetNozzleBuzzer);

#undef SCALE_PARAM