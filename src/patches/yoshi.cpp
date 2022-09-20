#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "common_sdk.h"
#include "module.hxx"
#include "player.hxx"
#include "stage.hxx"

using namespace BetterSMS;

#if BETTER_SMS_YOSHI_SAVE_NOZZLES

static bool isYoshiMaintainFluddModel() {
    TMario *player;
    SMS_FROM_GPR(31, player);

    auto playerData = Player::getData(player);

    if (!playerData)
        return player->mAttributes.mHasFludd;

    if (player->mYoshi->mState == TYoshi::MOUNTED)
        return (playerData->mFluddHistory.mHadFludd & player->mAttributes.mHasFludd);
    else
        return player->mAttributes.mHasFludd;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024D68C, 0x80245418, 0, 0), isYoshiMaintainFluddModel);
SMS_WRITE_32(SMS_PORT_REGION(0x8024D690, 0x8024541c, 0, 0), 0x2C030000);

static void saveNozzles(TYoshi *yoshi) {
    TMario *player = yoshi->mMario;

    auto playerData = Player::getData(player);

    if (!playerData->isMario()) {
        ride__6TYoshiFv(yoshi);
        return;
    }

    playerData->mFluddHistory.mMainNozzle   = player->mFludd->mCurrentNozzle;
    playerData->mFluddHistory.mSecondNozzle = player->mFludd->mSecondNozzle;
    playerData->mFluddHistory.mWaterLevel   = player->mFludd->mCurrentWater;
    playerData->mFluddHistory.mHadFludd     = player->mAttributes.mHasFludd;
    ride__6TYoshiFv(yoshi);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8028121C, 0x80278FA8, 0, 0), saveNozzles);

static void restoreNozzles(TMario *player) {
    auto playerData = Player::getData(player);

    if (!playerData->isMario())
        return;

    f32 factor = static_cast<f32>(playerData->mFluddHistory.mWaterLevel) /
                 static_cast<f32>(
                     player->mFludd->mNozzleList[playerData->mFluddHistory.mMainNozzle]->mMaxWater);
    changeNozzle__9TWaterGunFQ29TWaterGun11TNozzleTypeb(player->mFludd,
                                                        playerData->mFluddHistory.mSecondNozzle, 1);
    normalizeNozzle__6TMarioFv(player);
    player->mFludd->mCurrentWater =
        player->mFludd->mNozzleList[(u8)player->mFludd->mCurrentNozzle]->mMaxWater * factor;
    player->mAttributes.mHasFludd = playerData->mFluddHistory.mHadFludd;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024EC18, 0x802469A4, 0, 0), restoreNozzles);
SMS_WRITE_32(SMS_PORT_REGION(0x8024EC2C, 0x802469A8, 0, 0), 0x60000000);

#endif

#if BETTER_SMS_GREEN_YOSHI

SMS_WRITE_32(SMS_PORT_REGION(0x8026E9DC, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8026F14C, 0, 0, 0), 0x60000000);

#endif