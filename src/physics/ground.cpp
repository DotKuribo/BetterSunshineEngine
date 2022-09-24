#include <Dolphin/MTX.h>
#include <Dolphin/math.h>
#include <Dolphin/types.h>

#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/macros.h>
#include <SMS/npc/BaseNPC.hxx>

#include "collision/warp.hxx"
#include "common_sdk.h"
#include "geometry.hxx"
#include "module.hxx"
#include "player.hxx"

using namespace BetterSMS;

// extern -> SME.cpp
// 0x8025B8C0
static f32 checkGroundSpeedLimit() {
    TMario *player;
    SMS_FROM_GPR(31, player);

    auto playerData = Player::getData(player);

    f32 multiplier = 1.0f;
    if (player->onYoshi()) {
        multiplier *= player->mYoshiParams.mRunYoshiMult.get();
    } else {
        multiplier *= playerData->getParams()->mSpeedMultiplier.get();
    }
    return multiplier;
}
SMS_WRITE_32(SMS_PORT_REGION(0x8025B8BC, 0x80253648, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025B8C0, 0x8025364C, 0, 0), checkGroundSpeedLimit);
SMS_WRITE_32(SMS_PORT_REGION(0x8025B8C4, 0x80253650, 0, 0), 0xEFFF0072);

static f32 checkSlideSpeedMulti() {
    TMario *player;
    SMS_FROM_GPR(30, player);

    auto playerData = Player::getData(player);

    constexpr f32 speedCap         = 100.0f;
    constexpr f32 rocketMultiplier = 1.8f;
    constexpr f32 hoverMultiplier  = 1.2f;
    constexpr f32 brakeRate        = 0.005f;

    if (player->mFludd && isEmitting__9TWaterGunFv(player->mFludd)) {
        if (player->mFludd->mCurrentNozzle == TWaterGun::Hover ||
            player->mFludd->mCurrentNozzle == TWaterGun::Underwater)
            playerData->mSlideSpeedMultiplier = hoverMultiplier;
        else if (player->mFludd->mCurrentNozzle == TWaterGun::Rocket) {
            const f32 multiplier =
                (rocketMultiplier * ((speedCap * rocketMultiplier) / player->mForwardSpeed));
            playerData->mSlideSpeedMultiplier = rocketMultiplier;
            player->mPrevSpeed.scale(multiplier);
        } else {
            playerData->mSlideSpeedMultiplier =
                Max(playerData->mSlideSpeedMultiplier - brakeRate, 1.0f);
        }
    } else {
        playerData->mSlideSpeedMultiplier =
            Max(playerData->mSlideSpeedMultiplier - brakeRate, 1.0f);
    }

    if (!player->onYoshi()) {
        return speedCap * playerData->mSlideSpeedMultiplier;
    } else {
        return speedCap;
    }
}
SMS_WRITE_32(SMS_PORT_REGION(0x8025C3D8, 0x80254164, 0, 0), 0x40810028);
SMS_WRITE_32(SMS_PORT_REGION(0x8025C3FC, 0x80254188, 0, 0), 0xFC800018);
SMS_WRITE_32(SMS_PORT_REGION(0x8025C400, 0x8025418C, 0, 0), 0xD09E00B0);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025C404, 0x80254190, 0, 0), checkSlideSpeedMulti);
SMS_WRITE_32(SMS_PORT_REGION(0x8025C408, 0x80254194, 0, 0), 0xFC400890);
SMS_WRITE_32(SMS_PORT_REGION(0x8025C410, 0x8025419C, 0, 0), 0x60000000);

/* GOOP WALKING CODE */

// extern -> SME.cpp
// 0x8024E288
static void checkGraffitiAffected(TMario *player) {
    auto playerData = Player::getData(player);

    if (!playerData->isMario()) {
        player->checkGraffito();
    } else if (playerData->getParams()->mGoopAffected.get()) {
        player->checkGraffito();
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024E288, 0x80246014, 0, 0), checkGraffitiAffected);