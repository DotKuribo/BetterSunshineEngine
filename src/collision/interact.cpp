#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <SMS/SMS.hxx>
#include <SMS/collision/MapCollisionData.hxx>
#include <SMS/macros.h>
#include <SMS/sound/MSoundSESystem.hxx>

#include "collision/warp.hxx"
#include "common_sdk.h"
#include "globals.hxx"
#include "libs/constmath.hxx"
#include "logging.hxx"
#include "math.hxx"
#include "module.hxx"
#include "player.hxx"

#if BETTER_SMS_EXTRA_COLLISION

#pragma region SimpleCollisions

using namespace BetterSMS;
using namespace BetterSMS::Math;
using namespace BetterSMS::Collision;

static void elecPlayer(TMario *player, u8 flags) {
    if (player->mSubState == 0 && player->mState != 0x20338) {
        player->mHealth -= flags - 1;
        changePlayerStatus__6TMarioFUlUlb(player, 0x20338, 0, false);
    }
    if (gpMSound->gateCheck(0x1814)) {
        MSoundSE::startSoundActor(0x1814, reinterpret_cast<Vec *>(&player->mPosition), 0, nullptr,
                                  0, 4);
    }
    if (gpMSound->gateCheck(0x3806)) {
        MSoundSE::startSoundActor(0x3806, reinterpret_cast<Vec *>(&player->mPosition), 0, nullptr,
                                  0, 4);
    }
}

static void burnPlayer(TMario *player, u8 flags) {
    /*
     */

    changePlayerDropping__6TMarioFUlUl(player, 0x20464, 0);
    decHP__6TMarioFi(player, flags);

    dropObject__6TMarioFv(player);
    changePlayerStatus__6TMarioFUlUlb(player, 0x208B7, 1, false);
    player->mSpeed.y += 20.0f;
    emitAndBindToPosPtr__21TMarioParticleManagerFlPCQ29JGeometry8TVec3_f(
        *(u32 *)SMS_PORT_REGION(0x8040E150, 0x80405818, 0, 0), 6,
        reinterpret_cast<Vec *>(&player->mPosition), 0, nullptr);
    if (gpMSound->gateCheck(0x1813)) {
        MSoundSE::startSoundActor(0x1813, reinterpret_cast<Vec *>(&player->mPosition), 0, nullptr,
                                  0, 4);
    }
}

static void slipFloor(TMario *player, u8 flags) {
    const f32 strengthRun = scaleLinearAtAnchor<f32>(static_cast<f32>(flags), 0.001f, 1.0f);
    const f32 strengthSlide =
        Min(scaleLinearAtAnchor<f32>(static_cast<f32>(flags), 0.001f, 1.0f), 1.00001f);

#define SCALE_PARAM(param, scale) param.set(param.get() * scale)
    SCALE_PARAM(player->mDirtyParams.mBrakeStartValRun, strengthRun);
    SCALE_PARAM(player->mDirtyParams.mBrakeStartValSlip, strengthSlide);
    player->mDirtyParams.mPolSizeFootPrint.set(0.0f);
    player->mDirtyParams.mPolSizeJump.set(0.0f);
    player->mDirtyParams.mPolSizeRun.set(0.0f);
    player->mDirtyParams.mPolSizeSlip.set(0.0f);
    player->mDirtyParams.mDirtyMax.set(0.0f);
#undef SCALE_PARAM

    checkGraffitoSlip__6TMarioFv(player);
}

static void decHealth(TMario *player, u8 flags) { decHP__6TMarioFi(player, flags); }

static void incHealth(TMario *player, u8 flags) { incHP__6TMarioFi(player, flags); }

/* extern to handlers.cpp */

// Array of basic action functions bound to collision values
void (*gStateCBMap[])(TMario *player, u8 flags){elecPlayer, burnPlayer, slipFloor, decHealth,
                                                incHealth};
size_t gStateCBMapSize = sizeof(gStateCBMap) / sizeof(void *);

void checkIsGlideBounce(TMario *player) {
    auto playerData = Player::getData(player);

    if ((player->mFloorTriangle->mCollisionType & 0x7FFF) == 16007 ||
        (player->mFloorTriangle->mCollisionType & 0x7FFF) == 17007) {
        TBGCheckData *_oldCol   = player->mFloorTriangle;
        u16 _oldType            = _oldCol->mCollisionType;
        _oldCol->mCollisionType = 7;

        checkEnforceJump__6TMarioFv(player);
        _oldCol->mCollisionType = _oldType;

        playerData->mCollisionFlags.mIsSpinBounce = true;
        changePlayerStatus__6TMarioFUlUlb(player, static_cast<u32>(TMario::STATE_JUMPSPINR), 0, 0);
    } else
        checkEnforceJump__6TMarioFv(player);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80258334, 0x802500C0, 0, 0), checkIsGlideBounce);
SMS_PATCH_BL(SMS_PORT_REGION(0x80264CFC, 0x8025CA88, 0, 0), checkIsGlideBounce);

u16 checkIsRestoreTypeNoFallDamage(TBGCheckData *floor) {
    if ((floor->mCollisionType & 0x7FFF) == 16010 || (floor->mCollisionType & 0x7FFF) == 17010)
        return 10;
    else
        return floor->mCollisionType;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024C558, 0x802442E4, 0, 0), checkIsRestoreTypeNoFallDamage);

void checkRestoreHealth(TMario *player) {
    auto playerData = Player::getData(player);

    if (playerData->mCollisionTimer <= 0) {
        incHP__6TMarioFi(player, 1);
        playerData->mCollisionTimer = player->mFloorTriangle->mValue4;
    } else
        playerData->mCollisionTimer -= 1;
}

void checkIsCannonType(TMario *player) {
    auto playerData = Player::getData(player);

    if (!(player->mController->mButtons.mInput & TMarioGamePad::EButtons::DPAD_UP) ||
        !playerData->mCollisionFlags.mIsFaceUsed)
        return;

    if ((player->mFloorTriangle->mCollisionType & 0x7FFF) == 16080 ||
        (player->mFloorTriangle->mCollisionType & 0x7FFF) == 17080) {
        changePlayerStatus__6TMarioFUlUlb(player, static_cast<u32>(TMario::STATE_TRIPLE_J), 0, 0);
        emitParticle__6TMarioFis(player, static_cast<u32>(TMario::EFFECT_GROUND_SHARP_SHOCK));
        emitParticle__6TMarioFis(player, static_cast<u32>(TMario::EFFECT_GROUND_SMOKE_PLUME));
        player->mForwardSpeed = (u8)(player->mFloorTriangle->mValue4 >> 8);

        {
            Vec position;
            player->JSGGetTranslation(&position);
            position.y += 1.0f;
            player->JSGSetTranslation(position);
        }

        player->mSpeed.y                            = (u8)player->mFloorTriangle->mValue4;
        playerData->mCollisionFlags.mIsDisableInput = true;
        player->mController->State.mReadInput       = false;
        playerData->mCollisionFlags.mIsFaceUsed     = true;
    }
}

void changeNozzleType(TMario *player, u16 type) {
    auto playerData  = Player::getData(player);
    TWaterGun *fludd = player->mFludd;

    if (playerData->mCollisionFlags.mIsFaceUsed || !fludd)
        return;

    if (!playerData->getCanUseFludd())
        return;

    player->mAttributes.mHasFludd = player->mFloorTriangle->mValue4 == 1;

    TWaterGun::NozzleType nozzle = TWaterGun::Hover;
    if (type >= 17090)
        nozzle = static_cast<TWaterGun::NozzleType>(type - 17090);
    else
        nozzle = static_cast<TWaterGun::NozzleType>(type - 16090);

    if (fludd->mCurrentNozzle != nozzle) {
        changeNozzle__9TWaterGunFQ29TWaterGun11TNozzleTypeb(player->mFludd, nozzle, true);
        emitGetEffect__6TMarioFv(player);
    } else if (fludd->mCurrentWater < fludd->mNozzleList[nozzle]->mMaxWater) {
        emitGetWaterEffect__6TMarioFv(player);
    }

    fludd->mCurrentWater                    = fludd->mNozzleList[nozzle]->mMaxWater;
    playerData->mCollisionFlags.mIsFaceUsed = true;
}

void boostPadCol(TMario *player) {
    auto playerData = Player::getData(player);

    const f32 newSpeed    = player->mFloorTriangle->mValue4;
    const f32 scale       = newSpeed / player->mForwardSpeed;
    player->mForwardSpeed = newSpeed;
    player->mPrevSpeed.scale(scale);

    u32 targetState = (player->mState == static_cast<u32>(TMario::STATE_DIVESLIDE) ||
                       player->mState == static_cast<u32>(TMario::STATE_GOOPSLIDE))
                          ? player->mState
                          : static_cast<u32>(TMario::STATE_RUNNING);
    if (player->mState == static_cast<u32>(TMario::STATE_IDLE) ||
        !playerData->mCollisionFlags.mIsFaceUsed) {
        changePlayerStatus__6TMarioFUlUlb(player, targetState, 0, 0);
        startVoice__6TMarioFUl(player, static_cast<u32>(TMario::VOICE_JUMP));
    }
}

#pragma endregion

#pragma region WarpCollisions

// SMS_NO_INLINE void warpToLinkedCol(TMario *player, WarpKind kind, bool isInstantlyActivated) {
//     constexpr s32 DisableMovementTime = 80;
//     constexpr s32 TeleportTime        = 140;
//     constexpr s32 EnableMovementTime  = 60;
//     constexpr f32 WipeKindInDelay     = 1.0f;
//
//     auto playerData = Player::getData(player);
//
//     TBGCheckData *linkedCol =
//         BetterSMS::sWarpColArray->resolveCollisionWarp(player->mFloorTriangle);
//
//     const f32 speed = PSVECMag(reinterpret_cast<Vec *>(&player->mSpeed));
//
//     if (playerData->mIsWarpActive) {
//         switch (playerData->mWarpKind) {
//         case WarpKind::SPARKLES: {
//             if (playerData->mCollisionTimer > EnableMovementTime) {
//                 playerData->mCollisionFlags.mIsDisableInput = false;
//                 playerData->mIsWarpActive                   = false;
//                 player->mController->State.mReadInput       = true;
//                 playerData->mCollisionTimer                 = 0;
//             } else {
//                 playerData->mCollisionTimer += 1;
//             }
//             break;
//         }
//         case WarpKind::WIPE: {
//             if (gpApplication.mFader->mFadeStatus == TSMSFader::FADE_OFF) {
//                 playerData->mCollisionFlags.mIsDisableInput = false;
//                 playerData->mIsWarpActive                   = false;
//                 player->mController->State.mReadInput       = true;
//                 playerData->mCollisionTimer                 = 0;
//             } else {
//                 playerData->mCollisionTimer += 1;
//             }
//             break;
//         }
//         case WarpKind::INSTANT:
//         default:
//             playerData->mCollisionFlags.mIsDisableInput = false;
//             playerData->mIsWarpActive                   = false;
//         }
//     } else {
//         if (!linkedCol) {
//             playerData->mCollisionTimer = 0;
//             return;
//         }
//
//         switch (kind) {
//         case WarpKind::SPARKLES: {
//             size_t timeCut = 0;
//             if (isInstantlyActivated) {
//                 timeCut = DisableMovementTime;
//             } else if (speed > 1.0f) {
//                 playerData->mCollisionTimer = 0;
//                 return;
//             }
//
//             if (!playerData->mCollisionFlags.mIsFaceUsed) {
//                 if (playerData->mCollisionTimer >= TeleportTime - timeCut) {
//                     Player::warpToCollisionFace(player, linkedCol, false);
//
//                     playerData->mWarpKind                   = kind;
//                     playerData->mCollisionFlags.mIsFaceUsed = true;
//                     playerData->mIsWarpActive               = true;
//                     playerData->mCollisionFlags.mIsWarpUsed = true;
//                     playerData->mCollisionTimer             = 0;
//                     startSoundActor__6TMarioFUl(player, static_cast<u32>(TMario::VOICE_JUMP));
//                 } else if (playerData->mCollisionTimer >= DisableMovementTime - timeCut) {
//                     if (!playerData->mCollisionFlags.mIsDisableInput) {
//                         emitGetEffect__6TMarioFv(player);
//                     }
//                     playerData->mCollisionFlags.mIsDisableInput = true;
//                     player->mController->State.mReadInput       = false;
//                 }
//             }
//             playerData->mCollisionTimer += 1;
//             return;
//         }
//         case WarpKind::WIPE: {
//             size_t timeCut = 0;
//             if (isInstantlyActivated) {
//                 timeCut = DisableMovementTime;
//             } else if (speed > 1.0f) {
//                 playerData->mCollisionTimer = 0;
//                 return;
//             }
//
//             if (!playerData->mCollisionFlags.mIsFaceUsed) {
//                 if (gpApplication.mFader->mFadeStatus == TSMSFader::FADE_ON) {
//                     Player::warpToCollisionFace(player, linkedCol, false);
//
//                     playerData->mWarpKind                   = kind;
//                     playerData->mCollisionFlags.mIsFaceUsed = true;
//                     playerData->mIsWarpActive               = true;
//                     playerData->mCollisionFlags.mIsWarpUsed = true;
//                     playerData->mCollisionTimer             = 0;
//                     sIsWiping                               = false;
//
//                     gpApplication.mFader->startWipe(TSMSFader::WipeRequest::FADE_CIRCLE_IN, 1.0f,
//                                                     WipeKindInDelay);
//                 } else if (playerData->mCollisionTimer >= DisableMovementTime - timeCut) {
//                     playerData->mCollisionFlags.mIsDisableInput = true;
//                     player->mController->State.mReadInput       = false;
//                     if (gpApplication.mFader->mFadeStatus == TSMSFader::FADE_OFF && !sIsWiping) {
//                         gpApplication.mFader->startWipe(TSMSFader::WipeRequest::FADE_SPIRAL_OUT,
//                                                         1.0f, 0.0f);
//                         MSoundSE::startSoundSystemSE(0x4859, 0, nullptr, 0);
//                         sIsWiping = true;
//                     }
//                 }
//             }
//             playerData->mCollisionTimer += 1;
//             return;
//         }
//         case WarpKind::INSTANT:
//         default: {
//             if (!playerData->mCollisionFlags.mIsFaceUsed) {
//                 Player::warpToCollisionFace(player, linkedCol, false);
//
//                 playerData->mWarpKind                   = kind;
//                 playerData->mCollisionFlags.mIsFaceUsed = true;
//                 playerData->mCollisionFlags.mIsWarpUsed = true;
//                 playerData->mCollisionTimer             = 0;
//             }
//             return;
//         }
//         }
//     }
// }
//
// void warpToLinkedColPreserve(TMario *player, bool fluid) {
//     auto playerData = Player::getData(player);
//
//     TBGCheckData *linkedCol =
//         BetterSMS::sWarpColArray->resolveCollisionWarp(player->mFloorTriangle);
//
//     if (!linkedCol)
//         return;
//
//     if (!playerData->mCollisionFlags.mIsFaceUsed) {
//         Player::warpToCollisionFace(player, linkedCol, true);
//     } else {
//         playerData->mCollisionFlags.mIsFaceUsed =
//             (!(player->mController->mButtons.mFrameInput & TMarioGamePad::EButtons::DPAD_DOWN) &&
//              !fluid);
//     }
// }

#pragma endregion

#endif