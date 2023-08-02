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


#include "libs/constmath.hxx"
#include "module.hxx"
#include "player.hxx"
#include "p_settings.hxx"
#include "libs/geometry.hxx"

#if BETTER_SMS_BUGFIXES

using namespace BetterSMS;
using namespace BetterSMS::Geometry;

static void normalizeHoverSlopeSpeed(f32 floorPos) {
    TMario *player;
    SMS_FROM_GPR(22, player);

    player->mTranslation.y = floorPos;

    if (!BetterSMS::areExploitsPatched())
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

    const f32 lookAtRatio = 2 * (Vector3::lookAtRatio(playerForward, floorNormal) - 0.5f);

    if (isnan(lookAtRatio))
        return;

    if (lookAtRatio < 0.0f) {
        player->mForwardSpeed =
            Max(player->mForwardSpeed,
                -10.0f * clamp(scaleLinearAtAnchor(slopeStrength, fabsf(lookAtRatio), 1.0f), 0.0f, 1.0f));
    } else {
        player->mForwardSpeed =
            Min(player->mForwardSpeed,
                10.0f * clamp(scaleLinearAtAnchor(slopeStrength, lookAtRatio, 1.0f), 0.0f, 1.0f));
    }
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

// Patch rocket rollouts and other spray exploits //

static void checkExecWaterGun(TWaterGun *fludd) {
    if (!BetterSMS::areExploitsPatched()) {
        fludd->emit();
        return;
    }

    if (fludd->mCurrentNozzle != TWaterGun::Hover && fludd->mCurrentNozzle != TWaterGun::Rocket) {
        fludd->emit();
        return;
    }

    auto *playerData = Player::getData(fludd->mMario);
    if (playerData->getCanSprayFludd()) {
        fludd->emit();
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024E548, 0x802462D4, 0, 0), checkExecWaterGun);

static void killTriggerNozzle() {
    TNozzleTrigger *nozzle;
    SMS_FROM_GPR(29, nozzle);

    nozzle->mSprayState = TNozzleTrigger::DEAD;

    if (!BetterSMS::areExploitsPatched())
        return;

    if (nozzle->mFludd->mCurrentNozzle == TWaterGun::Hover || nozzle->mFludd->mCurrentNozzle == TWaterGun::Rocket) {
        auto *playerData     = Player::getData(nozzle->mFludd->mMario);
        playerData->setCanSprayFludd(false);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8026C370, 0x802640FC, 0, 0), killTriggerNozzle);

// 0x80262580
// extern -> SME.cpp
static bool checkAirNozzle() {
    TMario *player;
    SMS_FROM_GPR(31, player);

    if (!BetterSMS::areExploitsPatched())
        return player->mState != static_cast<u32>(TMario::STATE_HOVER_F);

    if (player->mFludd->mCurrentNozzle != TWaterGun::Hover && player->mFludd->mCurrentNozzle != TWaterGun::Rocket)
        return player->mState != static_cast<u32>(TMario::STATE_HOVER_F);

    auto *playerData = Player::getData(player);
    return (!(player->mState & static_cast<u32>(TMario::STATE_AIRBORN)) || playerData->getCanSprayFludd()) &&
           player->mState != static_cast<u32>(TMario::STATE_HOVER_F);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80262580, 0x8025A30C, 0, 0), checkAirNozzle);
SMS_WRITE_32(SMS_PORT_REGION(0x80262584, 0x8025A310, 0, 0), 0x2C030000);

BETTER_SMS_FOR_CALLBACK void updateDeadTriggerState(TMario *player, bool isMario) {
    if (!isMario)
        return;

    if (!BetterSMS::areExploitsPatched())
        return;

    auto *playerData = Player::getData(player);
    bool isAlive     = playerData->getCanSprayFludd();
    isAlive |= SMS_IsMarioTouchGround4cm__Fv();
    isAlive |= (player->mState & TMario::STATE_WATERBORN);
    isAlive |= (player->mState == TMario::STATE_NPC_BOUNCE);
    isAlive |= (player->mState == 0x350 || player->mState == 0x10000357 ||
                player->mState == 0x10000358);  // Ropes
    isAlive |= (player->mState == 0x10100341);  // Pole Climb
    playerData->setCanSprayFludd(isAlive);
}

#undef SCALE_PARAM