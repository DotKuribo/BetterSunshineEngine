#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <SMS/SMS.hxx>
#include <SMS/collision/MapCollisionData.hxx>
#include <SMS/macros.h>

#include "collision/warp.hxx"
#include "common_sdk.h"
#include "globals.hxx"
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
        player->mController->State.mReadInput       = true;
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
        player->mController->State.mReadInput = false;
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

#if SME_EXTRA_COLLISION

using namespace SME;
using namespace Util::Math;

// Array of basic action functions bound to collision values
extern void (*gStateCBMap[])(TMario *player, u8 flags);
extern size_t gStateCBMapSize;

// Collision functions
extern void checkIsCannonType(TMario *);
extern void boostPadCol(TMario *);
extern void setGravityCol(TMario *);
extern void antiGravityCol(TMario *);
extern void warpToLinkedCol(TMario *, Enum::WarpKind, bool);
extern void warpToLinkedColPreserve(TMario *, bool);
extern void changeNozzleType(TMario *, u16);
extern void checkRestoreHealth(TMario *);

static void slipperyCatchingSoundCheck(u32 sound, const Vec *pos, u32 unk_1, JAISound **out,
                                       u32 unk_2, u8 unk_3) {
    TMario *player;
    SME_FROM_GPR(31, player);

    if (player->mFloorTriangle->mCollisionType == 16081 ||
        player->mFloorTriangle->mCollisionType == 17081)
        sound = 4105;

    MSoundSE::startSoundActor(sound, pos, unk_1, out, unk_2, unk_3);
}
SME_PATCH_BL(SME_PORT_REGION(0x8025932C, 0x802510B8, 0, 0), slipperyCatchingSoundCheck);
SME_WRITE_32(SME_PORT_REGION(0x802596C0, 0x8025144C, 0, 0), 0x60000000);

/* Master Handlers */

static TBGCheckData *masterGroundCollisionHandler() {
    TMario *player;
    SME_FROM_GPR(29, player);

    TBGCheckData *floorCol = player->mFloorTriangle;
    const u16 type         = floorCol->mCollisionType & 0x7FFF;
    switch (type) {
    case 16007:
    case 17007:
        checkRestoreHealth(player);
        break;
    case 16010:
    case 17010:
        checkRestoreHealth(player);
        break;
    case 16020:
    case 17020:
        boostPadCol(player);
        break;
    case 16021:
    case 17021:
        setGravityCol(player);
        break;
    case EXPAND_WARP_SET(16040):
    case EXPAND_WARP_SET(17040):
        warpToLinkedCol(player, Enum::WarpKind::SPARKLES, false);
        break;
    case EXPAND_WARP_SET(16041):
    case EXPAND_WARP_SET(17041):
        warpToLinkedColPreserve(player, false);
        break;
    case EXPAND_WARP_SET(16042):
    case EXPAND_WARP_SET(17042):
        warpToLinkedColPreserve(player, true);
        break;
    case EXPAND_WARP_SET(16043):
    case EXPAND_WARP_SET(17043):
        warpToLinkedCol(player, Enum::WarpKind::WIPE, false);
        break;
    case EXPAND_WARP_SET(16044):
    case EXPAND_WARP_SET(17044):
        warpToLinkedCol(player, Enum::WarpKind::INSTANT, true);
        break;
    case EXPAND_WARP_SET(16045):
    case EXPAND_WARP_SET(17045):
        warpToLinkedCol(player, Enum::WarpKind::SPARKLES, true);
        break;
    case EXPAND_WARP_SET(16046):
    case EXPAND_WARP_SET(17046):
        warpToLinkedCol(player, Enum::WarpKind::WIPE, true);
        break;
    case 16080:
    case 17080:
        checkIsCannonType(player);
        break;
    case 16081: {
        u16 value = player->mFloorTriangle->mValue4;
        u8 index  = u8(value >> 8);
        if (index < gStateCBMapSize)
            gStateCBMap[index](player, value);
        break;
    }
    case 17081: {
        u16 value = player->mFloorTriangle->mValue4;
        u8 index  = u8(value >> 8);
        if (index < gStateCBMapSize)
            gStateCBMap[index](player, value);
        break;
    }
    case 16090:
    case 16091:
    case 16092:
    case 16093:
    case 16094:
    case 16095:
    case 17090:
    case 17091:
    case 17092:
    case 17093:
    case 17094:
    case 17095:
        changeNozzleType(player, type);
        break;
    }
    return floorCol;
}
SME_PATCH_BL(SME_PORT_REGION(0x80250C9C, 0x80248328, 0, 0), masterGroundCollisionHandler);

static u32 masterAllCollisionHandler(TMario *player) {
    u16 type = player->mFloorTriangle->mCollisionType & 0x7FFF;
    switch (type) {
    case 16022:
    case 17022:
        setGravityCol(player);
        break;
    case 16023:
    case 17023:
        antiGravityCol(player);
        break;
    case 16190:
    case 16191:
    case 16192:
    case 16193:
    case 16194:
    case 16195:
    case 17190:
    case 17191:
    case 17192:
    case 17193:
    case 17194:
    case 17195:
        changeNozzleType(player, type);
        break;
    }
    return player->mState;
}
SME_PATCH_BL(SME_PORT_REGION(0x8025059C, 0x80248328, 0, 0), masterAllCollisionHandler);
SME_WRITE_32(SME_PORT_REGION(0x802505A0, 0x8024832C, 0, 0), 0x546004E7);

#undef EXPAND_WARP_SET
#undef EXPAND_WARP_CATEGORY

#endif

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
            player->mController->State.mReadInput       = true;
            playerData->mCollisionFlags.mIsDisableInput = false;
            playerData->mIsWarpActive                   = false;
        }
    }

    const f32 marioCollisionHeight = *(f32 *)SMS_PORT_REGION(0x80415CC4, 0x8040D21C, 0, 0) *
                                     playerData->getParams()->mSizeMultiplier.get();

    Vec playerPos;
    player->JSGGetTranslation(&playerPos);

    TBGCheckData *roofTri;

    f32 roofHeight = checkRoofPlane__6TMarioFRC3VecfPPC12TBGCheckData(
        player, playerPos, playerPos.y + (marioCollisionHeight / 4), &roofTri);

    if (!player->mAttributes.mIsGameOver) {
        if (roofHeight - player->mFloorBelow < (marioCollisionHeight - 40.0f) &&
            !(player->mState & static_cast<u32>(TMario::STATE_AIRBORN)) &&
            player->mState != static_cast<u32>(TMario::STATE_HANG) &&
            !isUnderWater__6TMarioCFv(player)) {
            playerData->mCollisionFlags.mCrushedTimer += 1;
        } else {
            playerData->mCollisionFlags.mCrushedTimer = 0;
        }

        if (playerData->mCollisionFlags.mCrushedTimer > CrushTimeToDie) {
            loserExec__6TMarioFv(player);
            playerData->mCollisionFlags.mCrushedTimer = 0;
        }
    }
}
