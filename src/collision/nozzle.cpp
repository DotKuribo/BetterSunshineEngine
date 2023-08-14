#include <Dolphin/mem.h>
#include <JSystem/JGeometry/JGMVec.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>

#include <SMS/GC2D/SMSFader.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Manager/MarioParticleManager.hxx>
#include <SMS/System/Application.hxx>

#include "libs/triangle.hxx"
#include "logging.hxx"
#include "player.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Collision;

static void changeNozzle(TMario *player, TWaterGun::TNozzleType kind, bool hasFludd,
                         bool showFill) {
    TWaterGun *fludd = player->mFludd;
    if (!fludd)
        return;

    player->mAttributes.mHasFludd = hasFludd;

    TNozzleBase *nozzle;
    if (fludd->mCurrentNozzle != fludd->mSecondNozzle)
        nozzle = fludd->mNozzleList[TWaterGun::Spray];
    else
        nozzle = fludd->mNozzleList[kind];

    if (fludd->mSecondNozzle != kind) {
        fludd->changeNozzle(kind, true);
    }

    if (fludd->mCurrentWater < nozzle->mEmitParams.mAmountMax.get() && showFill) {
        player->emitGetWaterEffect();
    }
    fludd->mCurrentWater = nozzle->mEmitParams.mAmountMax.get();
}

BETTER_SMS_FOR_CALLBACK void changeNozzleSpray(TMario *player, const TBGCheckData *data,
                                               u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::GROUNDED))
        return;

    if (!playerData->getCanUseFludd())
        return;

    changeNozzle(player, TWaterGun::Spray, data->mValue == 1,
                 (flags & Player::InteractionFlags::ON_ENTER) != 0);
    playerData->mCollisionFlags.mIsFaceUsed = true;
}

BETTER_SMS_FOR_CALLBACK void changeNozzleHover(TMario *player, const TBGCheckData *data,
                                               u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::GROUNDED))
        return;

    if (!playerData->getCanUseFludd())
        return;

    changeNozzle(player, TWaterGun::Hover, data->mValue == 1,
                 (flags & Player::InteractionFlags::ON_ENTER) != 0);
    playerData->mCollisionFlags.mIsFaceUsed = true;
}

BETTER_SMS_FOR_CALLBACK void changeNozzleTurbo(TMario *player, const TBGCheckData *data,
                                               u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::GROUNDED))
        return;

    if (!playerData->getCanUseFludd())
        return;

    changeNozzle(player, TWaterGun::Turbo, data->mValue == 1,
                 (flags & Player::InteractionFlags::ON_ENTER) != 0);
    playerData->mCollisionFlags.mIsFaceUsed = true;
}

BETTER_SMS_FOR_CALLBACK void changeNozzleRocket(TMario *player, const TBGCheckData *data,
                                                u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::GROUNDED))
        return;

    if (!playerData->getCanUseFludd())
        return;

    changeNozzle(player, TWaterGun::Rocket, data->mValue == 1,
                 (flags & Player::InteractionFlags::ON_ENTER) != 0);
    playerData->mCollisionFlags.mIsFaceUsed = true;
}

BETTER_SMS_FOR_CALLBACK void changeNozzleSprayOnTouch(TMario *player, const TBGCheckData *data,
                                                      u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::ON_ENTER))
        return;

    if (!playerData->getCanUseFludd())
        return;

    changeNozzle(player, TWaterGun::Spray, data->mValue == 1, true);
    playerData->mCollisionFlags.mIsFaceUsed = true;
}

BETTER_SMS_FOR_CALLBACK void changeNozzleHoverOnTouch(TMario *player, const TBGCheckData *data,
                                                      u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::ON_ENTER))
        return;

    if (!playerData->getCanUseFludd())
        return;

    changeNozzle(player, TWaterGun::Hover, data->mValue == 1, true);
    playerData->mCollisionFlags.mIsFaceUsed = true;
}

BETTER_SMS_FOR_CALLBACK void changeNozzleTurboOnTouch(TMario *player, const TBGCheckData *data,
                                                      u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::ON_ENTER))
        return;

    if (!playerData->getCanUseFludd())
        return;

    changeNozzle(player, TWaterGun::Turbo, data->mValue == 1, true);
    playerData->mCollisionFlags.mIsFaceUsed = true;
}

BETTER_SMS_FOR_CALLBACK void changeNozzleRocketOnTouch(TMario *player, const TBGCheckData *data,
                                                       u32 flags) {
    auto *playerData = Player::getData(player);

    if (!(flags & Player::InteractionFlags::ON_ENTER))
        return;

    if (!playerData->getCanUseFludd())
        return;

    changeNozzle(player, TWaterGun::Rocket, data->mValue == 1, true);
    playerData->mCollisionFlags.mIsFaceUsed = true;
}