#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <SMS/collision/MapCollisionData.hxx>
#include <SMS/macros.h>
#include <SMS/MSound/MSoundSESystem.hxx>

#include "collision/warp.hxx"
#include "common_sdk.h"
#include "logging.hxx"
#include "math.hxx"
#include "module.hxx"
#include "player.hxx"

using namespace BetterSMS;

/* Collision State Resetters */

#define EXPAND_WARP_SET(base) (base) : case ((base) + 10) : case ((base) + 20) : case ((base) + 30)
#define EXPAND_WARP_CATEGORY(base)                                                                 \
    (base) : case ((base) + 1) : case ((base) + 2) : case ((base) + 3) : case ((base) + 4)

static void resetValuesOnStateChange(TMario *player) {
    auto playerData = Player::getData(player);

    switch (player->mPrevState) {
    case static_cast<u32>(TMario::STATE_TRIPLE_J):
        playerData->mCollisionFlags.mIsDisableInput = false;
        player->mController->mState.mReadInput      = true;
        break;
    default:
        break;
    }

    switch (player->mState) {}

    if ((player->mState != static_cast<u32>(TMario::STATE_JUMPSPINR) &&
         player->mState != static_cast<u32>(TMario::STATE_JUMPSPINL)))
        playerData->mCollisionFlags.mIsSpinBounce = false;

    if (playerData->mCollisionFlags.mIsDisableInput)
        // Patches pausing/map escaping the controller lock
        player->mController->mState.mReadInput = false;
}

static void resetValuesOnGroundContact(TMario *player) {
    auto playerData = Player::getData(player);

    if ((player->mPrevState & static_cast<u32>(TMario::STATE_AIRBORN)) != 0 &&
        (player->mState & static_cast<u32>(TMario::STATE_AIRBORN)) == 0 &&
        playerData->mCollisionFlags.mIsAirborn) {
        playerData->mCollisionFlags.mIsAirborn      = false;
        playerData->mCollisionFlags.mIsDisableInput = false;
    }
}

static void resetValuesOnAirborn(TMario *player) {
    auto playerData = Player::getData(player);

    if ((player->mPrevState & static_cast<u32>(TMario::STATE_AIRBORN)) == 0 &&
        (player->mState & static_cast<u32>(TMario::STATE_AIRBORN)) != 0 &&
        !playerData->mCollisionFlags.mIsAirborn) {
        playerData->mCollisionFlags.mIsAirborn = true;
    }
}

static void resetValuesOnCollisionChange(TMario *player) {
    auto playerData = Player::getData(player);

    if (!player->mFloorTriangle || (player->mFloorTriangle == playerData->mPrevCollisionFloor))
        return;

    if (player->mFloorTriangle->mCollisionType != playerData->mPrevCollisionType) {
        playerData->mPrevCollisionType          = player->mFloorTriangle->mCollisionType;
        playerData->mPrevCollisionFloor         = player->mFloorTriangle;
        playerData->mCollisionFlags.mIsFaceUsed = false;
        playerData->mCollisionTimer             = 0;
    }

    switch (player->mFloorTriangle->mCollisionType) {
    case EXPAND_WARP_SET(16042):
    case EXPAND_WARP_SET(17042):
        playerData->setColliding(false);
        break;
    // Callback collision
    case 16081:
    case 17081: {
        u8 index = playerData->mPrevCollisionFloor->mValue4 >> 8;
        if (index == 2) {
            player->mDirtyParams = playerData->mDefaultDirtyParams;
        }
    }
    default:
        playerData->setColliding(true);
        break;
    }
}

#if BETTER_SMS_EXTRA_COLLISION

using namespace BetterSMS;

//static void slipperyCatchingSoundCheck(u32 sound, const Vec *pos, u32 unk_1, JAISound **out,
//                                       u32 unk_2, u8 unk_3) {
//    TMario *player;
//    SMS_FROM_GPR(31, player);
//
//    if (player->mFloorTriangle->mCollisionType == 16081 ||
//        player->mFloorTriangle->mCollisionType == 17081)
//        sound = 4105;
//
//    MSoundSE::startSoundActor(sound, pos, unk_1, out, unk_2, unk_3);
//}
//SMS_PATCH_BL(SMS_PORT_REGION(0x8025932C, 0x802510B8, 0, 0), slipperyCatchingSoundCheck);
//SMS_WRITE_32(SMS_PORT_REGION(0x802596C0, 0x8025144C, 0, 0), 0x60000000);

// extern -> generic.cpp
void updateCollisionContext(TMario *player) {
    constexpr s16 CrushTimeToDie = 0;

    auto playerData = Player::getData(player);

    resetValuesOnStateChange(player);
    resetValuesOnGroundContact(player);
    resetValuesOnAirborn(player);
    resetValuesOnCollisionChange(player);

    if (!Collision::TCollisionLink::isValidWarpCol(player->mFloorTriangle)) {
        if (playerData->mIsWarpActive) {
            player->mController->mState.mReadInput      = true;
            playerData->mCollisionFlags.mIsDisableInput = false;
            playerData->mIsWarpActive                   = false;
        }
    }

    const f32 marioCollisionHeight = *(f32 *)SMS_PORT_REGION(0x80415CC4, 0x8040D21C, 0, 0) *
                                     playerData->getParams()->mSizeMultiplier.get();

    Vec playerPos;
    player->JSGGetTranslation(&playerPos);

    TBGCheckData *roofTri;

    f32 roofHeight = player->checkRoofPlane(playerPos, playerPos.y + (marioCollisionHeight / 4),
                                            (const TBGCheckData **)&roofTri);

    if (!player->mAttributes.mIsGameOver) {
        if (roofHeight - player->mFloorBelow < (marioCollisionHeight - 40.0f) &&
            !(player->mState & static_cast<u32>(TMario::STATE_AIRBORN)) &&
            player->mState != static_cast<u32>(TMario::STATE_HANG) && !player->isUnderWater()) {
            playerData->mCollisionFlags.mCrushedTimer += 1;
        } else {
            playerData->mCollisionFlags.mCrushedTimer = 0;
        }

        if (playerData->mCollisionFlags.mCrushedTimer > CrushTimeToDie) {
            player->loserExec();
            playerData->mCollisionFlags.mCrushedTimer = 0;
        }
    }
}

#endif