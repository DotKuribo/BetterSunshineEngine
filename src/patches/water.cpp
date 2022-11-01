#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <JSystem/J2D/J2DPrint.hxx>
#include <JSystem/JKernel/JKRFileLoader.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/collision/MapCollisionData.hxx>
#include <SMS/mapobj/MapObjNormalLift.hxx>
#include <SMS/mapobj/MapObjTree.hxx>
#include <SMS/object/ResetFruit.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/sound/MSoundSESystem.hxx>

#include "common_sdk.h"
#include "module.hxx"
#include "p_settings.hxx"

using namespace BetterSMS;

#if BETTER_SMS_BUGFIXES || 1

static inline bool isColTypeWater(u16 type) { return (type > 255 && type < 261) || type == 16644; }

static void patchWaterDownWarp(f32 y) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    if (!BetterSMS::areBugsPatched()) {
        player->mPosition.y = y;
        return;
    }

    if (player->mFloorTriangleWater == player->mFloorTriangle &&
        !isColTypeWater(player->mFloorTriangle->mCollisionType))
        player->changePlayerStatus(TMario::STATE_FALL, 0, false);
    else
        player->mPosition.y = y;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80272710, 0x8026A49C, 0, 0), patchWaterDownWarp);

static bool canDiePlane(f32 floorY) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    Vec playerPos;
    player->JSGGetTranslation(&playerPos);

    if (!BetterSMS::areBugsPatched())
        return floorY > playerPos.y;

    return (floorY > playerPos.y) && !player->mAttributes.mIsGameOver;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024FB54, 0x802478E4, 0, 0), canDiePlane);
SMS_WRITE_32(SMS_PORT_REGION(0x8024FB58, 0x802478E8, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x8024FB5C, 0x802478EC, 0, 0), 0x41820084);

static f32 enhanceWaterCheck(f32 x, f32 y, f32 z, TMario *player) {
    SMS_FROM_GPR(29, player);

    const TBGCheckData **tri = const_cast<const TBGCheckData **>(&player->mFloorTriangleWater);
    const TMapCollisionData *mapCol = gpMapCollisionData;

    if (BetterSMS::areBugsPatched()) {
        if (!(player->mState & TMario::STATE_WATERBORN)) {
            f32 yPos = mapCol->checkGround(x, player->mCeilingAbove - 10.0f, z, 0, tri);
            if (*tri && isColTypeWater((*tri)->mCollisionType)) {
                // if (!(player->mState & TMario::STATE_AIRBORN) &&
                //     isColTypeWater(player->mFloorTriangle->mCollisionType)) {
                //     player->mFloorBelow = gpMapCollisionData->checkGround(
                //         player->mPosition.x, player->mPosition.y, player->mPosition.z, 1,
                //         &player->mFloorTriangle);
                //     player->mPosition.y = player->mFloorBelow;
                // }
                return yPos;
            }
        }
    }

    return mapCol->checkGround(x, y, z, 0, tri);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024F12C, 0x80246EB8, 0, 0), enhanceWaterCheck);

// static f32 enhanceWaterSurfaceRiding(f32 adjust, f32 groundHeight) {
//     TMario *player;
//     SMS_FROM_GPR(31, player);

//     if (player->mFloorTriangle == player->mFloorTriangleWater) {
//         const TBGCheckData *floor;

//         f32 height = gpMapCollisionData->checkGround(player->mPosition.x, player->mPosition.y,
//                                                      player->mPosition.z, 1, &floor);

//         if (floor != &gpMapCollisionData->mIllegalCheckData) {
//             groundHeight = height;
//         }

//         player->mPosition.y = Min(player->mPosition.y, player->mWaterHeight);
//     }

//     return adjust + groundHeight;
// }
// SMS_PATCH_BL(SMS_PORT_REGION(0x80272FB0, 0, 0, 0), enhanceWaterSurfaceRiding);
// SMS_WRITE_32(SMS_PORT_REGION(0x80272FB4, 0, 0, 0), 0xC01F00F0);
// SMS_WRITE_32(SMS_PORT_REGION(0x802732DC, 0, 0, 0), 0x60000000);

// static u16 checkWaterStanding(TBGCheckData *data) {
//     TMario *player;
//     SMS_FROM_GPR(29, player);

//     const u16 colType   = data->mCollisionType;
//     const u16 waterType = colType - 256;
//     if ((waterType >= 0 && waterType < 5) || colType == 16644) {
//         if (!(player->mState & TMario::STATE_AIRBORN) &&
//             !(player->mState & TMario::STATE_WATERBORN))
//             player->changePlayerStatus(TMario::STATE_FALL, 0, false);
//     }

//     return colType;
// }
// SMS_PATCH_BL(SMS_PORT_REGION(0x8024F134, 0, 0, 0), checkWaterStanding);

#endif