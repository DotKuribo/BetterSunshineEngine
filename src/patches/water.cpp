#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <JSystem/J2D/J2DPrint.hxx>
#include <JSystem/JKernel/JKRFileLoader.hxx>
#include <SMS/Camera/CubeManagerBase.hxx>
#include <SMS/Camera/CubeMapTool.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Manager/MarioParticleManager.hxx>
#include <SMS/Map/Map.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/MapObj/MapObjNormalLift.hxx>
#include <SMS/MapObj/MapObjTree.hxx>
#include <SMS/MapObj/MapObjWave.hxx>
#include <SMS/MoveBG/ResetFruit.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/raw_fn.hxx>

#include "libs/geometry.hxx"
#include "module.hxx"
#include "p_settings.hxx"
#include "player.hxx"

#define ENABLE_WATER_CAVE_CAMERA_TYPE 0x100

#if 1

using namespace BetterSMS;

static inline bool isColTypeWater(u16 type) { return (type > 255 && type < 261) || type == 16644; }

static void normalToRotationMatrix(const TVec3f &normal, Mtx out) {
    TVec3f up      = fabsf(normal.y) < 0.999f ? TVec3f::up() : TVec3f::forward();
    TVec3f forward = normal;

    TVec3f right;
    PSVECNormalize(forward, forward);
    PSVECCrossProduct(forward, up, right);
    PSVECNormalize(right, right);

    PSVECCrossProduct(forward, right, up);
    PSVECNormalize(up, up);

    PSMTXIdentity(out);

#if 0
    out[0][0] = right.x;
    out[0][1] = right.y;
    out[0][2] = right.z;

    out[1][0] = up.x;
    out[1][1] = up.y;
    out[1][2] = up.z;

    out[2][0] = forward.x;
    out[2][1] = forward.y;
    out[2][2] = forward.z;
#else
    out[0][0] = right.x;
    out[1][0] = right.y;
    out[2][0] = right.z;

    out[0][1] = up.x;
    out[1][1] = up.y;
    out[2][1] = up.z;

    out[0][2] = forward.x;
    out[1][2] = forward.y;
    out[2][2] = forward.z;
#endif
}

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

    // Static roofs
    f32 aboveY = data.checkRoofList(position.x, position.y, position.z, ignoreFlags,
                                    data.mStaticCollisionRoot[cellX + (cellZ * data.mBlockXCount)]
                                        .mCheckList[TBGCheckListRoot::ROOF]
                                        .mNextTriangle,
                                    out);

    // Dynamic roofs
    {
        const TBGCheckData *potential;
        f32 potentialY =
            data.checkRoofList(position.x, position.y, position.z, ignoreFlags,
                               data.mMoveCollisionRoot[cellX + (cellZ * data.mBlockXCount)]
                                   .mCheckList[TBGCheckListRoot::ROOF]
                                   .mNextTriangle,
                               &potential);

        if (potentialY < aboveY && potentialY > position.y) {
            *out   = potential;
            aboveY = potentialY;
        }
    }

    // Static walls
    {
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
    }

    // Dynamic walls
    {
        const TBGCheckData *potential;
        f32 potentialY =
            data.checkRoofList(position.x, position.y, position.z, ignoreFlags,
                               data.mMoveCollisionRoot[cellX + (cellZ * data.mBlockXCount)]
                                   .mCheckList[TBGCheckListRoot::WALL]
                                   .mNextTriangle,
                               &potential);

        if (potentialY < aboveY && potentialY > position.y) {
            *out   = potential;
            aboveY = potentialY;
        }
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

    // Static ground
    f32 aboveY = data.checkGroundList(position.x, position.y, position.z, ignoreFlags,
                                      data.mStaticCollisionRoot[cellX + (cellZ * data.mBlockXCount)]
                                          .mCheckList[TBGCheckListRoot::GROUND]
                                          .mNextTriangle,
                                      out);

    // Dynamic ground
    {
        const TBGCheckData *potential;
        f32 potentialY =
            data.checkGroundList(position.x, position.y, position.z, ignoreFlags,
                                 data.mMoveCollisionRoot[cellX + (cellZ * data.mBlockXCount)]
                                     .mCheckList[TBGCheckListRoot::GROUND]
                                     .mNextTriangle,
                                 &potential);

        if (potentialY > aboveY && potentialY <= position.y) {
            *out   = potential;
            aboveY = potentialY;
        }
    }

    // Static walls
    {
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
    }

    // Dynamic walls
    {
        const TBGCheckData *potential;
        f32 potentialY =
            data.checkGroundList(position.x, position.y, position.z, ignoreFlags,
                                 data.mMoveCollisionRoot[cellX + (cellZ * data.mBlockXCount)]
                                     .mCheckList[TBGCheckListRoot::WALL]
                                     .mNextTriangle,
                                 &potential);

        if (potentialY > aboveY && potentialY <= position.y) {
            *out   = potential;
            aboveY = potentialY;
        }
    }

    return aboveY;
}

// IMPORTANT: Does not always set the water pointer due to the nature of the function.
// PLEASE INITIALIZE TO NULLPTR FIRST
SMS_NO_INLINE static f32 enhanceWaterCheckPlayer_(TMario *player, f32 x, f32 y, f32 z, bool considerCave,
                                                  const TMap *map, const TBGCheckData **water) {
    const TVec3f samplePosition = {x, player->mTranslation.y + 80.0f, z};

    const TBGCheckData *potential;
    f32 roofY, potentialY;

    const TBGCheckData *roofPlane;
    roofY      = findAnyRoofLikePlaneAbove(samplePosition, *map->mCollisionData, 0, &roofPlane);
    potentialY = findAnyGroundLikePlaneBelow({samplePosition.x, roofY - 1.0f, samplePosition.z},
                                             *map->mCollisionData, 8, &potential);

    if (isColTypeWater(roofPlane->mType)) {
        // If it is ocean water let's just assume the player is in water
        if (roofPlane->mType == 258 || roofPlane->mType == 259) {
            *water = roofPlane;
            return roofY;
        }
        *water = potential;
        return potentialY;
    }

    if (potential != &TMapCollisionData::mIllegalCheckData) {
        // Since there is water below the roof, check if there is ground between the player
        // and the water
        f32 groundY =
            findAnyGroundLikePlaneBelow({samplePosition.x, potentialY - 10.0f, samplePosition.z},
                                        *map->mCollisionData, 1, &roofPlane);
        if (groundY <= samplePosition.y) {
            // If there is no ground between the player and the new water, we can just
            // return the new water level
            *water = potential;
            return potentialY;
        } else {
            if (roofY <= player->mWaterHeight) {
                return player->mWaterHeight;
            }
            player->mWaterHeight = player->mFloorBelow;
            *water               = player->mFloorTriangle;
            return player->mFloorBelow;
        }
    } else if (considerCave) {
        // If there is no water beneath the roof, check if there is water above the player
        // (cave setting)
        potentialY =
            findAnyGroundLikePlaneBelow({x, 10000000.0f, z}, *map->mCollisionData, 8, &potential);
        if (potential == &TMapCollisionData::mIllegalCheckData) {
            if (roofPlane == &TMapCollisionData::mIllegalCheckData) {
                player->mWaterHeight = player->mFloorBelow;
                *water               = player->mFloorTriangle;
                return player->mFloorBelow;
            }
            return player->mWaterHeight;
        }

        *water = potential;
        return Min(roofY + 100.0f, potentialY);
    } else {
        potentialY =
            findAnyGroundLikePlaneBelow({x, 10000000.0f, z}, *map->mCollisionData, 8, &potential);
        roofY = findAnyRoofLikePlaneAbove(samplePosition, *map->mCollisionData, 1, &roofPlane);
        if (roofY > potentialY && potential != &TMapCollisionData::mIllegalCheckData) {
            *water = potential;
            return potentialY;
        }
        player->mWaterHeight = player->mFloorBelow;
        *water               = player->mFloorTriangle;
        return player->mFloorBelow;
    }
}

// IMPORTANT: Does not always set the water pointer due to the nature of the function.
// PLEASE INITIALIZE TO NULLPTR FIRST
SMS_NO_INLINE f32 enhanceWaterCheckGeneric_(f32 x, f32 y, f32 z, bool considerCave, const TMap *map,
                                            const TBGCheckData **water) {
    const TVec3f samplePosition = {x, y + 80.0f, z};

    const TBGCheckData *potential;
    f32 roofY, potentialY;

    const TBGCheckData *roofPlane;
    roofY      = findAnyRoofLikePlaneAbove(samplePosition, *map->mCollisionData, 0, &roofPlane);
    potentialY = findAnyGroundLikePlaneBelow({samplePosition.x, roofY - 1.0f, samplePosition.z},
                                             *map->mCollisionData, 8, &potential);

    bool isRoofWater = roofPlane && isColTypeWater(roofPlane->mType);
    if (isRoofWater) {
        // If it is ocean water let's just assume the player is in water
        if (roofPlane->mType == 258 || roofPlane->mType == 259) {
            *water = roofPlane;
            return roofY;
        }
        *water = potential;
        return potentialY;
    }

    if (potential != &TMapCollisionData::mIllegalCheckData) {
        // Since there is water below the roof, check if there is ground between the player
        // and the water
        f32 groundY =
            findAnyGroundLikePlaneBelow({samplePosition.x, potentialY - 10.0f, samplePosition.z},
                                        *map->mCollisionData, 1, &roofPlane);
        if (groundY <= samplePosition.y) {
            // If there is no ground between the player and the new water, we can just
            // return the new water level
            *water = potential;
            return potentialY;
        } else {
            return findAnyGroundLikePlaneBelow({x, y, z}, *map->mCollisionData, 8, water);
        }
    } else if (considerCave) {
        // If there is no water beneath the roof, check if there is water above the player
        // (cave setting)
        potentialY =
            findAnyGroundLikePlaneBelow({x, 10000000.0f, z}, *map->mCollisionData, 8, &potential);
        if (potential == &TMapCollisionData::mIllegalCheckData) {
            // Prevent potential crash
            if (*water == &TMapCollisionData::mIllegalCheckData) {
                return potentialY;
            }
            return roofY + 10.0f;
        }

        *water = potential;
        return Min(roofY, potentialY);
    } else {
        potentialY =
            findAnyGroundLikePlaneBelow({x, 10000000.0f, z}, *map->mCollisionData, 8, &potential);
        roofY = findAnyRoofLikePlaneAbove(samplePosition, *map->mCollisionData, 1, &roofPlane);
        if (roofY > potentialY) {
            *water = potential;
            return potentialY;
        }
        return findAnyGroundLikePlaneBelow({x, y, z}, *map->mCollisionData, 8, water);
    }
}

static f32 enhanceWaterCheckAndStickToSurface(f32 x, f32 y, f32 z, const TMap *map,
                                              const TBGCheckData **water) {
    TMario *player;
    SMS_FROM_GPR(29, player);

    Player::TPlayerData *playerData = Player::getData(player);

    if (!BetterSMS::isCollisionRepaired() || SMS_isDivingMap__Fv()) {
        return map->checkGround(x, y, z, water);
    }

    bool considerCave = (player->mState & TMario::STATE_WATERBORN) != 0;
    f32 height        = enhanceWaterCheckPlayer_(player, x, y, z, considerCave, map, water);

    bool waterborn = player->mState & TMario::STATE_WATERBORN;
    if (waterborn) {
        f32 heightDiff = height - player->mTranslation.y;
        if (playerData->mIsSwimmingWaterSurface && player->mState != 0x24D9 &&
            player->mState != 0x24DF && heightDiff < 120.0f) {
            player->mTranslation.y = Max(player->mTranslation.y, height - 80.0f);
        }
        playerData->mIsSwimmingWaterSurface = (height - player->mTranslation.y) < 81.0f;
    }

    return height;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024F12C, 0x80246EB8, 0, 0), enhanceWaterCheckAndStickToSurface);

static f32 enhanceWaterSurfaceRiding(f32 adjust, f32 groundHeight) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    if (!BetterSMS::isCollisionRepaired()) {
        return adjust + groundHeight;
    }

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
                                           const TBGCheckData **out) {
    if (!BetterSMS::isCollisionRepaired()) {
        return map->checkGround(x, y, z, out);
    }
    return map->mCollisionData->checkGround(x, y, z, 32, out);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802510E8, 0, 0, 0), enhanceCheckGroundPlaneForWater);
SMS_PATCH_BL(SMS_PORT_REGION(0x80251168, 0, 0, 0), enhanceCheckGroundPlaneForWater);

static f32 enhanceMarioGroundPlaneCheck(TMap *map, f32 x, f32 y, f32 z, const TBGCheckData **out) {
    if (!BetterSMS::isCollisionRepaired()) {
        return map->checkGround(x, y, z, out);
    }
    return map->mCollisionData->checkGround(x, y, z, 16, out);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80250B54, 0, 0, 0), enhanceMarioGroundPlaneCheck);

static f32 makeFluddHoverNotClipWaterRoof(TMap *map, f32 x, f32 y, f32 z,
                                          const TBGCheckData **out) {
    TMario *player = gpMarioAddress;

    if (!BetterSMS::isCollisionRepaired()) {
        return map->checkRoof(x, y, z, out);
    }

    return map->mCollisionData->checkRoof(x, y, z, 1, out);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802555E8, 0, 0, 0), makeFluddHoverNotClipWaterRoof);

static void checkIfWaterSplashOnEntry(TMario *player, f32 waterHeight) {
    if ((waterHeight - player->mTranslation.y) > 50) {
        return;
    }
    player->inOutWaterEffect(waterHeight);
}

static bool checkIfHipAttackingValid(f32 groundY) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    if (!BetterSMS::isCollisionRepaired()) {
        return groundY > player->mTranslation.y;
    }

    if (isColTypeWater(player->mFloorTriangle->mType)) {
        return false;
    }

    return groundY > player->mTranslation.y;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024A7BC, 0, 0, 0), checkIfHipAttackingValid);
SMS_WRITE_32(SMS_PORT_REGION(0x8024A7C0, 0, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x8024A7C4, 0, 0, 0), 0x41820028);

static void checkIfObjGeneralWallCollisionIsWater(TMapObjGeneral *obj, TVec3f *out,
                                                  TBGWallCheckRecord *record) {
    if (BetterSMS::isCollisionRepaired() && isColTypeWater(obj->mWallTouching->mType)) {
        return;
    }

    out->x = record->mPosition.x;
    out->z = record->mPosition.z;
    obj->calcReflectingVelocity(record->mWalls[0],
                                obj->mObjData->mPhysicalInfo->mPhysicalData->mWallBounceSpeed,
                                &obj->mSpeed);
}
SMS_PATCH_B(SMS_PORT_REGION(0x801B3EE4, 0, 0, 0), checkIfObjGeneralWallCollisionIsWater);

static void checkIfObjBallWallCollisionIsWater(TMapObjBall *obj, TVec3f *out,
                                               TBGWallCheckRecord *record) {
    // Speed reference
    TVec3f &sp = obj->mSpeed;

    if ((obj->mStateFlags.asU32 & 0x80) == 0 && obj->mObjectID != 0x400000D0) {
        f32 speedMag = sqrtf(sp.x * sp.x + sp.y * sp.y + sp.z * sp.z);
        sp.y += obj->_184 * speedMag;
    }

    bool collisionRepaired = BetterSMS::isCollisionRepaired();

    for (size_t i = 0; i < record->mNumWalls; ++i) {
        const TBGCheckData *wall = record->mWalls[i];
        if (collisionRepaired && isColTypeWater(wall->mType)) {
            obj->touchWaterSurface();
            *out = obj->mTranslation;
            return;
        }

        f32 normDot = sp.x * wall->mNormal.x + sp.y * wall->mNormal.y + sp.z * wall->mNormal.z;
        if (normDot >= 0.0f) {
            continue;
        }

        f32 projDot = out->x * wall->mNormal.x + out->y * wall->mNormal.y +
                      out->z * wall->mNormal.z + wall->mProjectionFactor;
        f32 reflectDot =
            normDot * -(1.0f + obj->mObjData->mPhysicalInfo->mPhysicalData->mWallBounceSpeed);

        out->x += (obj->mMaxSpeed - projDot) * wall->mNormal.x;
        out->z += (obj->mMaxSpeed - projDot) * wall->mNormal.z;

        sp.x += reflectDot * wall->mNormal.x;
        sp.z += reflectDot * wall->mNormal.z;

        if (obj->mObjectID == 0x400000D0) {
            f32 speedMag = fabsf(sqrtf(sp.x * sp.x + sp.y * sp.y + sp.z * sp.z));
            if (obj->mScale.y < 5.0f) {
                if (gpMSound->gateCheck(0x308B)) {
                    MSoundSE::startSoundActorWithInfo(0x308B, obj->mTranslation, nullptr, speedMag,
                                                      0, 0, nullptr, 0, 4);
                }
            } else {
                if (gpMSound->gateCheck(0x308A)) {
                    MSoundSE::startSoundActorWithInfo(0x308A, obj->mTranslation, nullptr, speedMag,
                                                      0, 0, nullptr, 0, 4);
                }
            }
        } else {
            u32 soundID = obj->mObjData->mSoundInfo->mSoundData->mReboundSoundID;
            if (gpMSound->gateCheck(soundID)) {
                MSoundSE::startSoundActorWithInfo(soundID, obj->mTranslation, obj->mSpeed, 0, 0, 0,
                                                  nullptr, 0, 4);
            }
        }
    }
}
SMS_PATCH_B(SMS_PORT_REGION(0x801E5770, 0, 0, 0), checkIfObjBallWallCollisionIsWater);

static void checkIfObjGeneralRoofCollisionIsWater(TMapObjGeneral *obj, TVec3f *out) {
    if (BetterSMS::isCollisionRepaired() && isColTypeWater(obj->mRoofTouching->mType)) {
        return;
    }
    out->y = obj->_03;
}
SMS_PATCH_B(SMS_PORT_REGION(0x801B3DF0, 0, 0, 0), checkIfObjGeneralRoofCollisionIsWater);

static void checkIfObjBallRoofCollisionIsWater(TMapObjBall *obj, TVec3f *out) {
    if (BetterSMS::isCollisionRepaired() && isColTypeWater(obj->mRoofTouching->mType)) {
        obj->touchWaterSurface();
        *out = obj->mTranslation;
        return;
    }
    out->y = Max(out->y, obj->_03);
    obj->calcReflectingVelocity(obj->mRoofTouching,
                                obj->mObjData->mPhysicalInfo->mPhysicalData->mFloorBounceSpeed,
                                &obj->mSpeed);
}
SMS_PATCH_B(SMS_PORT_REGION(0x801E5AD4, 0, 0, 0), checkIfObjBallRoofCollisionIsWater);

static SMS_NO_INLINE void fixMarioOceanAnimBug_(TMario *player, J3DTransformInfo &info, Mtx out) {
    if (BetterSMS::isCollisionRepaired()) {
        if (gpMapObjWave) {
            const TBGCheckData *water = nullptr;
            f32 waterY                = findAnyGroundLikePlaneBelow(
                {player->mTranslation.x, 10000000.0f, player->mTranslation.z},
                *gpMap->mCollisionData, 8, &water);
            if (water != &TMapCollisionData::mIllegalCheckData) {
                f32 waveHeight =
                    gpMapObjWave->getWaveHeight(player->mTranslation.x, player->mTranslation.z);
                f32 scale = 1.0f / Max(1.0f, fabsf(waterY - player->mTranslation.y) / 100.0f);
                info.ty   = player->mTranslation.y + waveHeight * scale;
            }
        } else {
            info.ty = player->mTranslation.y;
        }
    }

    J3DGetTranslateRotateMtx(info, out);
}

static void fixMarioOceanAnimBug(J3DTransformInfo &info, Mtx out) {
    TMario *player;
    SMS_FROM_GPR(30, player);
    fixMarioOceanAnimBug_(player, info, out);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802456B8, 0, 0, 0), fixMarioOceanAnimBug);

static f32 fixBlooperSurfAnimBug(TMapObjWave *objWave, f32 x, f32 y, f32 z) {
    if (!BetterSMS::isCollisionRepaired()) {
        return objWave->getHeight(x, y, z);
    }

    if (!objWave) {
        return -32768.0f;
    }

    bool considerCave            = false;
    gpMarioAddress->mWaterHeight = enhanceWaterCheckPlayer_(gpMarioAddress, x, y, z, considerCave, gpMap,
                                                            &gpMarioAddress->mFloorTriangleWater);

    if (gpMarioAddress->mFloorTriangleWater && gpMarioAddress->mFloorTriangleWater->mType == 258 ||
        gpMarioAddress->mFloorTriangleWater->mType == 259) {
        return objWave->getWaveHeight(x, z);
    }

    return 0;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80245F6C, 0, 0, 0), fixBlooperSurfAnimBug);

static const TBGCheckData *fixBlooperParamDifferentiation() {
    TMario *player = gpMarioAddress;

    if (!BetterSMS::isCollisionRepaired()) {
        return player->mFloorTriangle;
    }

    return player->mFloorTriangleWater ? player->mFloorTriangleWater : player->mFloorTriangle;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8025B690, 0, 0, 0), fixBlooperParamDifferentiation);

static f32 fixWaterFilterHeightCalc(TMapObjWave *objWave, f32 x, f32 y, f32 z) {
    if (!BetterSMS::isCollisionRepaired()) {
        return objWave->getHeight(x, y, z);
    }

    TMario *player            = gpMarioAddress;
    Player::TPlayerData *data = Player::getData(player);

    bool considerCave = false;

    TNameRefPtrAryT<TCubeGeneralInfo> *generalCubes = gpCubeCamera->getCubeInfo<TCubeGeneralInfo>();

    if (generalCubes) {
        for (size_t i = 0; i < generalCubes->mChildren.size(); ++i) {
            if (!gpCubeCamera->isInCube({x, y, z}, i)) {
                continue;
            }

            if (gpCubeCamera->getDataNo(i) == ENABLE_WATER_CAVE_CAMERA_TYPE) {
                considerCave = data->mIsCameraInWater;
                break;
            }
        }
    }

    const TBGCheckData *water = nullptr;
    f32 height                = enhanceWaterCheckGeneric_(x, y, z, considerCave, gpMap, &water);

    if (!objWave) {
        data->mIsCameraInWater = height > y;
        return height;
    }

    if (water && water->mType == 258 || water->mType == 259) {
        height += Min(objWave->getWaveHeight(x, z), 0.0f);
    }

    data->mIsCameraInWater = height > y;
    return height;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80189DC0, 0, 0, 0), fixWaterFilterHeightCalc);

SMS_WRITE_32(SMS_PORT_REGION(0x801EA8E0, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801EA8E4, 0, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x801EA8F4, 0, 0, 0), fixWaterFilterHeightCalc);

// Enable wave filter updates elsewhere from Gelato and Sirena
//SMS_WRITE_32(SMS_PORT_REGION(0x801DCE14, 0, 0, 0), 0x60000000);
//SMS_WRITE_32(SMS_PORT_REGION(0x801DCE18, 0, 0, 0), 0x60000000);
//SMS_WRITE_32(SMS_PORT_REGION(0x801DCE1C, 0, 0, 0), 0x60000000);
//SMS_WRITE_32(SMS_PORT_REGION(0x801DCE20, 0, 0, 0), 0x60000000);
//SMS_WRITE_32(SMS_PORT_REGION(0x801DCE24, 0, 0, 0), 0x60000000);

BETTER_SMS_FOR_CALLBACK void initializeMapObjWave(TMarDirector* director) {
    gpMapObjWave = nullptr;  // lol
}

#endif