#include <Dolphin/mem.h>
#include <JSystem/JGeometry/JGMVec.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>

#include <SMS/System/Application.hxx>
#include <SMS/Manager/MarioParticleManager.hxx>
#include <SMS/GC2D/SMSFader.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>

#include "libs/warp.hxx"
#include "libs/triangle.hxx"
#include "logging.hxx"
#include "player.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Geometry;
using namespace BetterSMS::Collision;

static void changeNozzle(TMario *player, TWaterGun::TNozzleType kind) {
    TWaterGun *fludd = player->mFludd;
    if (!fludd)
        return;

    TNozzleBase *nozzle = fludd->mNozzleList[kind];
    if (fludd->mCurrentNozzle != kind) {
        fludd->changeNozzle(kind, true);
        player->emitGetEffect();
    } else if (fludd->mCurrentWater < nozzle->mEmitParams.mAmountMax.get()) {
        player->emitGetWaterEffect();
    }

    fludd->mCurrentWater = nozzle->mEmitParams.mAmountMax.get();
}

void changeNozzleSpray(TMario *player, const TBGCheckData *data, u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::ON_ENTER))
        return;

    TWaterGun *fludd = player->mFludd;
    if (!fludd)
        return;

    if (!playerData->getCanUseFludd())
        return;

    player->mAttributes.mHasFludd = data->mValue == 1;

    changeNozzle(player, TWaterGun::Spray);

    playerData->mCollisionFlags.mIsFaceUsed = true;
}

void changeNozzleHover(TMario *player, const TBGCheckData *data, u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::ON_ENTER))
        return;

    TWaterGun *fludd = player->mFludd;
    if (!fludd)
        return;

    if (!playerData->getCanUseFludd())
        return;

    player->mAttributes.mHasFludd = data->mValue == 1;

    changeNozzle(player, TWaterGun::Hover);

    playerData->mCollisionFlags.mIsFaceUsed = true;
}

void changeNozzleTurbo(TMario *player, const TBGCheckData *data, u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::ON_ENTER))
        return;

    TWaterGun *fludd = player->mFludd;
    if (!fludd)
        return;

    if (!playerData->getCanUseFludd())
        return;

    player->mAttributes.mHasFludd = data->mValue == 1;

    changeNozzle(player, TWaterGun::Turbo);

    playerData->mCollisionFlags.mIsFaceUsed = true;
}

void changeNozzleRocket(TMario *player, const TBGCheckData *data, u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::ON_ENTER))
        return;

    TWaterGun *fludd = player->mFludd;
    if (!fludd)
        return;

    if (!playerData->getCanUseFludd())
        return;

    player->mAttributes.mHasFludd = data->mValue == 1;

    changeNozzle(player, TWaterGun::Rocket);

    playerData->mCollisionFlags.mIsFaceUsed = true;
}