#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <JSystem/J2D/J2DPrint.hxx>
#include <JSystem/JKernel/JKRFileLoader.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Map/Map.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/MapObj/MapObjNormalLift.hxx>
#include <SMS/MapObj/MapObjTree.hxx>
#include <SMS/MoveBG/ResetFruit.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/raw_fn.hxx>

#include "module.hxx"
#include "p_settings.hxx"

using namespace BetterSMS;

static inline bool isColTypeWater(u16 type) { return (type > 255 && type < 261) || type == 16644; }

static void patchWaterDownWarp(f32 y) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    if (!BetterSMS::isCollisionRepaired()) {
        player->mTranslation.y = y;
        return;
    }

    if (fabsf(player->mTranslation.y - y) < 10.0f) {
        if (player->mFloorTriangleWater != &TMapCollisionData::mIllegalCheckData &&
            player->mFloorTriangleWater != player->mFloorTriangle) {
            player->mTranslation.y = y;
            return;
        }

        if (isColTypeWater(player->mFloorTriangle->mType)) {
            player->mTranslation.y = y;
            return;
        }
    }

    player->changePlayerStatus(TMario::STATE_FALL, 0, false);
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

static f32 findAnyRoofLikePlaneAbove(const TVec3f &position, TMapCollisionData &data,
                                     const TBGCheckData **out) {
    const f32 gridFraction = 1.0f / 1024.0f;

    const f32 boundsX = data.mAreaSizeX;
    const f32 boundsZ = data.mAreaSizeZ;

    *out = &TMapCollisionData::mIllegalCheckData;

    if (position.x < -boundsX || position.x >= boundsX)
        return 0;

    if (position.z < -boundsZ || position.z >= boundsZ)
        return 0;

    const int cellX = gridFraction * (position.x + boundsX);
    const int cellZ = gridFraction * (position.z + boundsZ);

    f32 aboveY = data.checkRoofList(position.x, position.y, position.z, 0,
                                    data.mStaticCollisionRoot[cellX + (cellZ * data.mBlockXCount)]
                                        .mCheckList[TBGCheckListRoot::ROOF]
                                        .mNextTriangle,
                                    out);

    const TBGCheckData *potential;
    f32 potentialY =
        data.checkRoofList(position.x, position.y, position.z, 0,
                           data.mStaticCollisionRoot[cellX + (cellZ * data.mBlockXCount)]
                               .mCheckList[TBGCheckListRoot::WALL]
                               .mNextTriangle,
                           &potential);

    if (potentialY < aboveY && potentialY > position.y) {
        *out    = potential;
        aboveY = potentialY;
    }

    return aboveY;
}

static bool isTriOccludedFromPoint(const TBGCheckData *tri, const TVec3f &point, f32 faceY) {
    if (tri == &TMapCollisionData::mIllegalCheckData) {
        return false;
    }

    if (faceY == point.y) {
        return false;
    }

    TVec3f projPoint = {point.x, faceY, point.z};

    const TBGCheckData *thePlane;
    if (faceY < point.y) {
        f32 roofY = findAnyRoofLikePlaneAbove(projPoint, *gpMapCollisionData, &thePlane);

        if (roofY < point.y) {
            OSReport("Roof is below point and thus occluded\n");
            return true;
        }

        f32 groundY = gpMapCollisionData->checkGround(point.x, point.y - 10.0f, point.z, 4, &thePlane);
        if (groundY > faceY) {
            OSReport("Ground is above water and thus occluded\n");
            return true;
        }
    } else {
        f32 roofY = findAnyRoofLikePlaneAbove(point, *gpMapCollisionData, &thePlane);

        if (roofY < faceY) {
            OSReport("Roof is below water and thus occluded\n");
            return true;
        }

        f32 groundY =
            gpMapCollisionData->checkGround(projPoint.x, projPoint.y - 10.0f, projPoint.z, 4, &thePlane);
        if (groundY > point.y) {
            OSReport("Ground is above point and thus occluded\n");
            return true;
        }
    }

    return false;
}

static f32 enhanceWaterCheck(f32 x, f32 y, f32 z, const TMap *map, const TBGCheckData **water) {
    TMario *player              = gpMarioAddress;
    const TVec3f samplePosition = {x, player->mTranslation.y, z};

    const TBGCheckData *waterPlane, *potential;
    f32 roofY, potentialY;

    //if (BetterSMS::isCollisionRepaired() && (player->mState & TMario::STATE_WATERBORN)) {
    //    // There is no water above the player, so do sanity checks for air swimming (cave
    //    // setting, roof above player)
    //    const TBGCheckData *roofPlane;
    //    roofY      = findAnyRoofLikePlaneAbove(samplePosition, *map->mCollisionData, &roofPlane);
    //    f32 waterY = map->mCollisionData->checkGround(x, roofY, z, 8, &waterPlane);

    //    OSReport("RoofY: %f, WaterY: %f\n", roofY, waterY);

    //    if (isColTypeWater(waterPlane->mType)) {
    //        OSReport("Water plane is water\n");
    //        if (!isTriOccludedFromPoint(waterPlane, samplePosition, waterY)) {
    //            OSReport("Water plane is not occluded\n");
    //            *water = waterPlane;
    //            return waterY;
    //        }
    //    }

    //    if (roofPlane != &TMapCollisionData::mIllegalCheckData) {
    //        OSReport("Roof plane is not illegal\n");
    //        *water = roofPlane;
    //        return roofY;
    //    }

    //    OSReport("Roof plane is illegal\n");
    //}

    //if (BetterSMS::isCollisionRepaired() && !SMS_isDivingMap__Fv()) {
    //    if (!(player->mState & TMario::STATE_WATERBORN)) {
    //        const TBGCheckData *tmpPlane;
    //        roofY      = findAnyRoofLikePlaneAbove(samplePosition, *map->mCollisionData, &tmpPlane);
    //        potentialY = map->mCollisionData->checkGround(samplePosition.x, roofY - 10.0f,
    //                                                      samplePosition.z, 8, &potential);
    //        if (potential != &TMapCollisionData::mIllegalCheckData) {
    //            // Since there is water below the roof, check if there is ground between the player
    //            // and the water
    //            roofY = map->mCollisionData->checkGround(samplePosition.x, potentialY - 10.0f,
    //                                                     samplePosition.z, 0, &tmpPlane);
    //            if (roofY <= samplePosition.y) {
    //                // If there is no ground between the player and the new water, we can just
    //                // return the new water level
    //                *water = potential;
    //                return potentialY;
    //            }
    //        }
    //    }
    //}

    if (BetterSMS::isCollisionRepaired()) {
        const TBGCheckData *tmpPlane;
        roofY      = findAnyRoofLikePlaneAbove(samplePosition, *map->mCollisionData, &tmpPlane);
        // Find water plane beneath the roof, (hopefully) above the player.
        potentialY = map->mCollisionData->checkGround(samplePosition.x, roofY - 10.0f, samplePosition.z,
                                                      8, &potential);
        if (potential != &TMapCollisionData::mIllegalCheckData) {
            // Since there is water below the roof, check if there is ground between the player
            // and the water
            roofY = map->mCollisionData->checkGround(samplePosition.x, potentialY - 10.0f,
                                                     samplePosition.z, 0, &tmpPlane);
            if (roofY <= samplePosition.y) {
                // If there is no ground between the player and the new water, we can just
                // return the new water level
                *water = potential;
                return potentialY;
            }
        } else {
            // If there is no water beneath the roof, check if there is water above the player
            // (cave setting)
            return map->mCollisionData->checkGround(x, 1000000.0f, z, 8, water);
        }
    }

    // Passing 8, filters to just water when collision is fixed
    // This is the default behavior if all exotic checks fail
    return map->mCollisionData->checkGround(x, y, z, 8, water);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024F12C, 0x80246EB8, 0, 0), enhanceWaterCheck);

// static f32 enhanceWaterSurfaceRiding(f32 adjust, f32 groundHeight) {
//     TMario *player;
//     SMS_FROM_GPR(31, player);

//     if (player->mFloorTriangle == player->mFloorTriangleWater) {
//         const TBGCheckData *floor;

//         f32 height = gpMapCollisionData->checkGround(player->mTranslation.x,
//         player->mTranslation.y,
//                                                      player->mTranslation.z, 1, &floor);

//         if (floor != &gpMapCollisionData->mIllegalCheckData) {
//             groundHeight = height;
//         }

//         player->mTranslation.y = Min(player->mTranslation.y, player->mWaterHeight);
//     }

//     return adjust + groundHeight;
// }
// SMS_PATCH_BL(SMS_PORT_REGION(0x80272FB0, 0, 0, 0), enhanceWaterSurfaceRiding);
// SMS_WRITE_32(SMS_PORT_REGION(0x80272FB4, 0, 0, 0), 0xC01F00F0);
// SMS_WRITE_32(SMS_PORT_REGION(0x802732DC, 0, 0, 0), 0x60000000);

// static u16 checkWaterStanding(TBGCheckData *data) {
//     TMario *player;
//     SMS_FROM_GPR(29, player);

//     const u16 colType   = data->mType;
//     const u16 waterType = colType - 256;
//     if ((waterType >= 0 && waterType < 5) || colType == 16644) {
//         if (!(player->mState & TMario::STATE_AIRBORN) &&
//             !(player->mState & TMario::STATE_WATERBORN))
//             player->changePlayerStatus(TMario::STATE_FALL, 0, false);
//     }

//     return colType;
// }
// SMS_PATCH_BL(SMS_PORT_REGION(0x8024F134, 0, 0, 0), checkWaterStanding);
