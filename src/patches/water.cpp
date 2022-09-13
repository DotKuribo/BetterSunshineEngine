#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <JSystem/J2D/J2DPrint.hxx>
#include <JSystem/JKernel/JKRFileLoader.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/SMS.hxx>
#include <SMS/actor/Mario.hxx>
#include <SMS/mapobj/MapObjNormalLift.hxx>
#include <SMS/mapobj/MapObjTree.hxx>
#include <SMS/object/ResetFruit.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/sound/MSoundSESystem.hxx>

#include "common_sdk.h"
#include "module.hxx"

#include "module.hxx"

using namespace BetterSMS;

#if BETTER_SMS_BUGFIXES

static void patchWaterDownWarp(f32 y) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    if (player->mFloorTriangleWater == player->mFloorTriangle)
        changePlayerStatus__6TMarioFUlUlb(player, TMario::STATE_FALL, 0, false);
    else
        player->mPosition.y = y;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80272710, 0x8026A49C, 0, 0), patchWaterDownWarp);

static bool canDiePlane(f32 floorY) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    Vec playerPos;
    player->JSGGetTranslation(&playerPos);

    return (floorY > playerPos.y) && !player->mAttributes.mIsGameOver;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024FB54, 0x802478E4, 0, 0), canDiePlane);
SMS_WRITE_32(SMS_PORT_REGION(0x8024FB58, 0x802478E8, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x8024FB5C, 0x802478EC, 0, 0), 0x41820084);

static f32 enhanceWaterCheck(f32 x, f32 y, f32 z, TMario *player) {
    SMS_FROM_GPR(29, player);

    const TBGCheckData **tri = const_cast<const TBGCheckData **>(&player->mFloorTriangleWater);

    const TMapCollisionData *mapCol = gpMapCollisionData;
    if (!(player->mState & TMario::STATE_SWIM)) {
        f32 yPos = mapCol->checkGround(x, player->mCeilingAbove - 10.0f, z, 0, tri);
        if (*tri && (*tri)->mCollisionType > 255 && (*tri)->mCollisionType < 260)
            return yPos;
    }

    return mapCol->checkGround(x, y, z, 0, tri);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024F12C, 0x80246EB8, 0, 0), enhanceWaterCheck);

#endif

#if BETTER_SMS_UNDERWATER_FRUIT

// 0x801E542C
// extern -> SME.cpp
static bool canFruitDieWater(TResetFruit *fruit) {
    if (fruit->mObjectID == TResetFruit::DURIAN) {
        fruit->touchWaterSurface();
        return true;
    } else {
        fruit->mStateFlags.asFlags.mHasPhysics = true;
        if (gpMSound->gateCheck(14453)) {
            Vec fruitPos;
            fruit->JSGGetTranslation(&fruitPos);
            fruit->emitColumnWater();
            MSoundSESystem::MSoundSE::startSoundActor(14453, &fruitPos, 0, 0, 0, 4);
        }
    }
    return false;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801E542C, 0x801DD304, 0, 0), canFruitDieWater);
SMS_WRITE_32(SMS_PORT_REGION(0x801E5430, 0x801DD308, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x801E5434, 0x801DD30C, 0, 0), 0x41820140);

// 0x8023F964
static f32 chooseGrabDistancing(M3UModelMario *model) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    if (player->mPrevState & static_cast<u32>(TMario::STATE_WATERBORN)) {
        SMS_TO_GPR(3, model);
        return 0.0f;
    } else {
        SMS_TO_GPR(3, model);
        return 11.0f;
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8023F964, 0x802376F0, 0, 0), chooseGrabDistancing);

// 0x8023F9DC
static bool isGrabWaitOver(TMario *player) {
    return isLast1AnimeFrame__6TMarioFv(player) |
           (player->mPrevState & static_cast<u32>(TMario::STATE_WATERBORN));
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8023F9DC, 0x80237768, 0, 0), isGrabWaitOver);

#endif