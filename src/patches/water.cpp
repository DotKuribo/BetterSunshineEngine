#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <JSystem/J2D/J2DPrint.hxx>
#include <JSystem/JKernel/JKRFileLoader.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Manager/MarioParticleManager.hxx>
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

    if (!BetterSMS::isCollisionRepaired() || !(player->mState & TMario::STATE_WATERBORN)) {
        player->mTranslation.y = y;
        return;
    }

    if (fabsf(player->mTranslation.y - y) < 100.0f) {
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
                                     u8 ignoreFlags, const TBGCheckData **out) {
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

    f32 aboveY = data.checkRoofList(position.x, position.y, position.z, ignoreFlags,
                                    data.mStaticCollisionRoot[cellX + (cellZ * data.mBlockXCount)]
                                        .mCheckList[TBGCheckListRoot::ROOF]
                                        .mNextTriangle,
                                    out);

    const TBGCheckData *potential;
    f32 potentialY =
        data.checkRoofList(position.x, position.y, position.z, ignoreFlags,
                           data.mStaticCollisionRoot[cellX + (cellZ * data.mBlockXCount)]
                               .mCheckList[TBGCheckListRoot::WALL]
                               .mNextTriangle,
                           &potential);

    if (potentialY < aboveY && potentialY > position.y) {
        *out   = potential;
        aboveY = potentialY;
    }

    return aboveY;
}

static f32 findAnyGroundLikePlaneBelow(const TVec3f &position, TMapCollisionData &data,
                                       u8 ignoreFlags, const TBGCheckData **out) {
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

    f32 aboveY = data.checkGroundList(position.x, position.y, position.z, ignoreFlags,
                                      data.mStaticCollisionRoot[cellX + (cellZ * data.mBlockXCount)]
                                          .mCheckList[TBGCheckListRoot::GROUND]
                                          .mNextTriangle,
                                      out);

    const TBGCheckData *potential;
    f32 potentialY =
        data.checkGroundList(position.x, position.y, position.z, ignoreFlags,
                             data.mStaticCollisionRoot[cellX + (cellZ * data.mBlockXCount)]
                                 .mCheckList[TBGCheckListRoot::WALL]
                                 .mNextTriangle,
                             &potential);

    if (potentialY > aboveY && potentialY <= position.y) {
        *out   = potential;
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
        f32 roofY = findAnyRoofLikePlaneAbove(projPoint, *gpMapCollisionData, 0, &thePlane);

        if (roofY < point.y) {
            return true;
        }

        f32 groundY = findAnyGroundLikePlaneBelow({point.x, point.y - 10.0f, point.z},
                                                  *gpMapCollisionData, 4, &thePlane);
        if (groundY > faceY) {
            return true;
        }
    } else {
        f32 roofY = findAnyRoofLikePlaneAbove(point, *gpMapCollisionData, 0, &thePlane);

        if (roofY < faceY) {
            return true;
        }

        f32 groundY = findAnyGroundLikePlaneBelow({projPoint.x, projPoint.y - 10.0f, projPoint.z},
                                                  *gpMapCollisionData, 4, &thePlane);
        if (groundY > point.y) {
            return true;
        }
    }

    return false;
}

static f32 enhanceWaterCheck(f32 x, f32 y, f32 z, const TMap *map, const TBGCheckData **water) {
    TMario *player              = gpMarioAddress;
    const TVec3f samplePosition = {x, player->mTranslation.y + 80.0f, z};

    if (!BetterSMS::isCollisionRepaired()) {
        return map->mCollisionData->checkGround(x, y, z, 0, water);
    }

    const TBGCheckData *potential;
    f32 roofY, potentialY;

    const TBGCheckData *roofPlane;
    roofY      = findAnyRoofLikePlaneAbove(samplePosition, *map->mCollisionData, 0, &roofPlane);
    potentialY = findAnyGroundLikePlaneBelow({samplePosition.x, roofY - 1.0f, samplePosition.z},
                                             *map->mCollisionData, 8, &potential);

    bool isRoofWater = roofPlane && isColTypeWater(roofPlane->mType) && roofY > samplePosition.y;
    if (isRoofWater) {
        return findAnyGroundLikePlaneBelow(samplePosition, *map->mCollisionData, 8, water);
    }

    if (potential != &TMapCollisionData::mIllegalCheckData) {
        // Since there is water below the roof, check if there is ground between the player
        // and the water
        f32 groundY =
            findAnyGroundLikePlaneBelow({samplePosition.x, potentialY - 10.0f, samplePosition.z},
                                        *map->mCollisionData, 0, &roofPlane);
        if (groundY <= samplePosition.y) {
            // If there is no ground between the player and the new water, we can just
            // return the new water level
            *water = potential;
            return potentialY;
        } else {
            return map->mCollisionData->checkGround(x, y, z, 8, water);
        }
    } else if ((player->mState & TMario::STATE_WATERBORN)) {
        // If there is no water beneath the roof, check if there is water above the player
        // (cave setting)
        return findAnyGroundLikePlaneBelow({x, 10000000.0f, z}, *map->mCollisionData, 8, water);
    } else {
        potentialY =
            findAnyGroundLikePlaneBelow({x, 10000000.0f, z}, *map->mCollisionData, 8, &potential);
        roofY = findAnyRoofLikePlaneAbove(samplePosition, *map->mCollisionData, 1, &roofPlane);
        if (roofY > potentialY) {
            *water = potential;
            return potentialY;
        }
        return findAnyGroundLikePlaneBelow({x, y, z}, *map->mCollisionData, 8, &potential);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024F12C, 0x80246EB8, 0, 0), enhanceWaterCheck);

static f32 enhanceWaterSurfaceRiding(f32 adjust, f32 groundHeight) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    if (player->mFloorTriangle == player->mFloorTriangleWater) {
        const TBGCheckData *floor;

        f32 height = gpMapCollisionData->checkGround(player->mTranslation.x, player->mTranslation.y,
                                                     player->mTranslation.z, 1, &floor);

        if (floor != &gpMapCollisionData->mIllegalCheckData) {
            groundHeight = height;
        }

        player->mTranslation.y = Min(player->mTranslation.y, player->mWaterHeight);
    }

    return adjust + groundHeight;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80272FB0, 0, 0, 0), enhanceWaterSurfaceRiding);
SMS_WRITE_32(SMS_PORT_REGION(0x80272FB4, 0, 0, 0), 0xC01F00F0);
SMS_WRITE_32(SMS_PORT_REGION(0x802732DC, 0, 0, 0), 0x60000000);

static f32 enhanceCheckGroundPlaneForWater(TMap *map, f32 x, f32 y, f32 z,
                                           const TBGCheckData **faceOut) {
    return map->mCollisionData->checkGround(x, y, z, 1, faceOut);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802510E8, 0, 0, 0), enhanceCheckGroundPlaneForWater);
SMS_PATCH_BL(SMS_PORT_REGION(0x80251168, 0, 0, 0), enhanceCheckGroundPlaneForWater);

static f32 makeFluddHoverNotClipWaterRoof(TMap *map, f32 x, f32 y, f32 z,
                                          const TBGCheckData **out) {
    TMario *player = gpMarioAddress;

    if (!BetterSMS::areExploitsPatched())
        return map->checkRoof(x, y, z, out);

    return map->mCollisionData->checkRoof(x, y, z, 1, out);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802555E8, 0, 0, 0), makeFluddHoverNotClipWaterRoof);

static void normalToRotationMatrix(Mtx out, const TVec3f &normal) {
    TVec3f up = TVec3f::up();
    TVec3f right;
    up.cross(normal, right);
    right.normalize();

    TVec3f forward;
    normal.cross(right, forward);
    forward.normalize();

    PSMTXIdentity(out);
    out[0][0] = right.x;
    out[1][0] = right.y;
    out[2][0] = right.z;

    out[0][1] = up.x;
    out[1][1] = up.y;
    out[2][1] = up.z;

    out[0][2] = forward.x;
    out[1][2] = forward.y;
    out[2][2] = forward.z;
}

static void emitExoticInOutWaterEffect(TMarioParticleManager *manager, s32 effect,
                                       const TVec3f *pos, u8 count, const void *owner) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    bool isExitingBottom = player->mRoofTriangle && isColTypeWater(player->mRoofTriangle->mType) &&
                           !((player->mRoofTriangle->mMinHeight - player->mTranslation.y) > 50.0f);

    bool isExitingTop = (player->mWaterHeight - player->mTranslation.y) < 50.0f;

    bool isExitingWall = player->mWallTriangle && player->mWaterHeight > player->mTranslation.y;

    Mtx44 mtx;
    PSMTXIdentity(mtx);
    PSMTXTrans(mtx, pos->x, player->mWaterHeight, pos->z);

    Mtx44 lookMtx;
    if (isExitingBottom) {
        normalToRotationMatrix(lookMtx, player->mRoofTriangle->mNormal);
    } else if (isExitingTop) {
        normalToRotationMatrix(lookMtx, player->mFloorTriangleWater->mNormal);
    } else if (isExitingWall) {
        normalToRotationMatrix(lookMtx, player->mWallTriangle->mNormal);
    } else {
        return;
    }

    PSMTXConcat(mtx, lookMtx, mtx);

    manager->emitAndBindToMtx(effect, mtx, count, owner);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80264278, 0, 0, 0), emitExoticInOutWaterEffect);
SMS_PATCH_BL(SMS_PORT_REGION(0x802642EC, 0, 0, 0), emitExoticInOutWaterEffect);
SMS_PATCH_BL(SMS_PORT_REGION(0x80264304, 0, 0, 0), emitExoticInOutWaterEffect);
SMS_PATCH_BL(SMS_PORT_REGION(0x8026431C, 0, 0, 0), emitExoticInOutWaterEffect);
SMS_PATCH_BL(SMS_PORT_REGION(0x80264358, 0, 0, 0), emitExoticInOutWaterEffect);
SMS_PATCH_BL(SMS_PORT_REGION(0x80264370, 0, 0, 0), emitExoticInOutWaterEffect);
SMS_PATCH_BL(SMS_PORT_REGION(0x80264388, 0, 0, 0), emitExoticInOutWaterEffect);

static JPABaseEmitter *emitExoticRippleWaterEffect(TMarioParticleManager *manager, s32 effect,
                                                   Mtx emitMtx, u8 count, const void *owner) {
    TMario *player = gpMarioAddress;

    bool isExitingTop = fabsf(player->mWaterHeight - player->mTranslation.y) < 100.0f;

    bool isExitingWall = player->mWallTriangle && player->mWaterHeight > player->mTranslation.y;

    Mtx44 mtx;
    PSMTXIdentity(mtx);
    PSMTXTrans(mtx, player->mTranslation.x, player->mWaterHeight, player->mTranslation.z);

    Mtx44 lookMtx;
    if (isExitingTop) {
        normalToRotationMatrix(lookMtx, player->mFloorTriangleWater->mNormal);
    } else if (isExitingWall) {
        normalToRotationMatrix(lookMtx, player->mWallTriangle->mNormal);
    } else {
        return manager->emitAndBindToMtxPtr(effect, emitMtx, count, owner);
    }

    PSMTXConcat(mtx, lookMtx, emitMtx);

    return manager->emitAndBindToMtxPtr(effect, emitMtx, count, owner);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80233BD4, 0, 0, 0), emitExoticRippleWaterEffect);
SMS_PATCH_BL(SMS_PORT_REGION(0x80233BF8, 0, 0, 0), emitExoticRippleWaterEffect);
SMS_PATCH_BL(SMS_PORT_REGION(0x80233C1C, 0, 0, 0), emitExoticRippleWaterEffect);

static void emitExoticRunningRippleWaterEffect(TMarioParticleManager *manager, s32 effect,
                                               const TVec3f *pos, u8 count, const void *owner) {
    TMario *player = gpMarioAddress;

    bool isExitingTop = (player->mWaterHeight - player->mTranslation.y) < 50.0f;

    bool isExitingWall = player->mWallTriangle && player->mWaterHeight > player->mTranslation.y;

    Mtx44 mtx;
    PSMTXIdentity(mtx);
    PSMTXTrans(mtx, pos->x, pos->y, pos->z);

    Mtx44 lookMtx;
    if (isExitingTop) {
        normalToRotationMatrix(lookMtx, player->mFloorTriangleWater->mNormal);
    } else if (isExitingWall) {
        normalToRotationMatrix(lookMtx, player->mWallTriangle->mNormal);
    } else {
        return;
    }

    PSMTXConcat(mtx, lookMtx, mtx);

    manager->emitAndBindToMtx(effect, mtx, count, owner);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80263F1C, 0, 0, 0), emitExoticRunningRippleWaterEffect);
SMS_PATCH_BL(SMS_PORT_REGION(0x80233D2C, 0, 0, 0), emitExoticRunningRippleWaterEffect);
SMS_PATCH_BL(SMS_PORT_REGION(0x80233D50, 0, 0, 0), emitExoticRunningRippleWaterEffect);

static void updateExoticWaterSplashEffect(Mtx src, Mtx dst) {
    TMario *player = gpMarioAddress;

    bool isExitingTop = (player->mWaterHeight - player->mTranslation.y) < 50.0f;

    bool isExitingWall = player->mWallTriangle && player->mWaterHeight > player->mTranslation.y;

    Mtx44 lookMtx;
    if (isExitingTop) {
        normalToRotationMatrix(lookMtx, player->mFloorTriangleWater->mNormal);
    } else if (isExitingWall) {
        normalToRotationMatrix(lookMtx, player->mWallTriangle->mNormal);
    } else {
        // Make the effect disappear lol xd
        PSMTXTrans(dst, 0, -100000.0f, 0);
        return;
    }

    PSMTXConcat(src, lookMtx, src);
    PSMTXCopy(src, dst);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80271E24, 0, 0, 0), updateExoticWaterSplashEffect);
SMS_PATCH_BL(SMS_PORT_REGION(0x8027203C, 0, 0, 0), updateExoticWaterSplashEffect);

static void checkIfWaterSplashOnEntry(TMario *player, f32 waterHeight) {
    if ((waterHeight - player->mTranslation.y) > 50) {
        return;
    }
    player->inOutWaterEffect(waterHeight);
}

static bool checkIfHipAttackingValid(f32 groundY) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    if (isColTypeWater(player->mFloorTriangle->mType)) {
        return false;
    }

    return groundY > player->mTranslation.y;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024A7BC, 0, 0, 0), checkIfHipAttackingValid);
SMS_WRITE_32(SMS_PORT_REGION(0x8024A7C0, 0, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x8024A7C4, 0, 0, 0), 0x41820028);
