#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "common_sdk.h"
#include "module.hxx"
#include "player.hxx"
#include "stage.hxx"
#include "yoshi.hxx"

using namespace BetterSMS;

#if BETTER_SMS_GREEN_YOSHI

SMS_WRITE_32(SMS_PORT_REGION(0x8026E068, 0x80265DF4, 0, 0),
             0x2C000001);  // Turn green when out of juice
SMS_WRITE_32(SMS_PORT_REGION(0x8026E0A0, 0x80265E2C, 0, 0),
             0x60000000);  // No flickering
SMS_WRITE_32(SMS_PORT_REGION(0x8026EE14, 0x80266BA0, 0, 0),
             0x4800020C);  // Yoshi can't starve

// 0x8026EB00, 0x8026EBFC, 0x8026F218
// extern -> SMS.cpp
static bool isYoshiDie(TMario *player) { return !Yoshi::isGreenYoshi(player); }
SMS_PATCH_BL(SMS_PORT_REGION(0x8026EB00, 0x80266BDC, 0, 0), isYoshiDie);
SMS_WRITE_32(SMS_PORT_REGION(0x8026EB04, 0x80266BE0, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x8026EB08, 0x80266BE4, 0, 0), 0x41820518);

SMS_PATCH_BL(SMS_PORT_REGION(0x8026EBFC, 0x80266988, 0, 0), isYoshiDie);
SMS_WRITE_32(SMS_PORT_REGION(0x8026EC00, 0x8026698C, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x8026EC04, 0x80266990, 0, 0), 0x4182041C);

SMS_PATCH_BL(SMS_PORT_REGION(0x8026F218, 0x80266FA4, 0, 0), isYoshiDie);
SMS_WRITE_32(SMS_PORT_REGION(0x8026F21C, 0x80266FA8, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x8026F220, 0x80266FAC, 0, 0), 0x41820164);

// 0x8024F240
// extern -> SMS.cpp
static void maybeYoshiDrown(TYoshi *yoshi) {
    if (!Yoshi::isGreenYoshi(yoshi))
        disappear__6TYoshiFv(yoshi);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024F240, 0x80246FCC, 0, 0), maybeYoshiDrown);

// 0x802810F8
// extern -> SMS.cpp
static bool canMountYoshi() {
    TMario *player;
    SMS_FROM_GPR(31, player);

    auto playerData = Player::getData(player);
    auto *params                    = playerData->getParams();

    if (params->mSizeMultiplier.get() *
            Stage::getStageConfiguration()->mPlayerSizeMultiplier.get() >
        1.5f)
        return false;

    if (player->mState & static_cast<u32>(TMario::STATE_WATERBORN))
        return params->mCanRideYoshi.get();
    else
        return ((player->mState & static_cast<u32>(TMario::STATE_AIRBORN)) &&
                params->mCanRideYoshi.get());
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802810F8, 0x80278E84, 0, 0), canMountYoshi);
SMS_WRITE_32(SMS_PORT_REGION(0x802810FC, 0x80278E88, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x80281100, 0x80278E8C, 0, 0), 0x807F03F0);
SMS_WRITE_32(SMS_PORT_REGION(0x80281104, 0x80278E90, 0, 0), 0x38830020);
SMS_WRITE_32(SMS_PORT_REGION(0x80281110, 0x80278E9C, 0, 0), 0x60000000);

static f32 getYoshiYPos(TYoshi *yoshi) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    return player->mYoshi->mCoordinates.y;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80281148, 0x80278ED4, 0, 0), getYoshiYPos);

static void canYoshiSpray(TWaterGun *gpWaterGun) {
    TMario *player = gpWaterGun->mMario;
    if (!player->mYoshi)
        return;

    if (!Yoshi::isGreenYoshiMounted(player->mYoshi))
        emit__9TWaterGunFv(gpWaterGun);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024E58C, 0x80246318, 0, 0), canYoshiSpray);

static u32 calcYoshiSwimVelocity(TMario *player, u32 arg1) {
    auto playerData = Player::getData(player);

    if (!playerData) {
        return jumpProcess__6TMarioFi(player, arg1);
    }

    if (Stage::getStageConfiguration()->mIsYoshiHungry.get())
        return jumpProcess__6TMarioFi(player, arg1);

    if (!player->mYoshi)
        return jumpProcess__6TMarioFi(player, arg1);

    if (!Yoshi::isGreenYoshiMounted(player->mYoshi))
        return jumpProcess__6TMarioFi(player, arg1);

    if (player->mController->mButtons.mInput & TMarioGamePad::EButtons::A) {
        if (playerData->mYoshiWaterSpeed.y > 12.0f)
            playerData->mYoshiWaterSpeed.y = 12.0f;
        else
            playerData->mYoshiWaterSpeed.y += 0.34375f;
    } else {
        if (playerData->mYoshiWaterSpeed.y < -12.0f)
            playerData->mYoshiWaterSpeed.y = -12.0f;
        else
            playerData->mYoshiWaterSpeed.y -= 0.34375f;
    }
    player->mSpeed.y = playerData->mYoshiWaterSpeed.y;
    return jumpProcess__6TMarioFi(player, arg1);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80273198, 0x8026AF24, 0, 0), calcYoshiSwimVelocity);

static u32 isYoshiWaterFlutter() {
    TYoshi *yoshi;
    u32 animID;
    SMS_FROM_GPR(29, yoshi);
    SMS_FROM_GPR(30, animID);

    if (!Stage::getStageConfiguration()->mIsYoshiHungry.get() &&
        Yoshi::isGreenYoshiAscendingWater(yoshi->mMario))
        animID = 9;

    if ((animID & 0xFFFF) == 24)
        animID = 15;

    return animID;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80270078, 0x80267E04, 0, 0), isYoshiWaterFlutter);
SMS_WRITE_32(SMS_PORT_REGION(0x8027007C, 0x80267E08, 0, 0), 0x7C7E1B78);
SMS_WRITE_32(SMS_PORT_REGION(0x80270080, 0x80267E0C, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80270084, 0x80267E10, 0, 0), 0x60000000);

// 0x8026FE84 NEEDS ADDI R4, R3, 0
static u32 isYoshiValidWaterFlutter(s32 anmIdx, u32 unk1, TMario *player) {
    if (!Stage::getStageConfiguration()->mIsYoshiHungry.get())
        return player->mState;

    if (Yoshi::isGreenYoshiAscendingWater(player))
        return (player->mState & 0xFFFFFBFF) | static_cast<u32>(TMario::STATE_AIRBORN);
    else
        return player->mState;
}
// SMS_PATCH_BL(SMS_PORT_REGION(0x8026FE84, 0, 0, 0), isYoshiValidWaterFlutter);

static bool isYoshiValidDrip(TYoshi *yoshi) {
    return Yoshi::isMounted(yoshi) && !Yoshi::isGreenYoshi(yoshi);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024E788, 0x80246514, 0, 0), isYoshiValidDrip);

#endif