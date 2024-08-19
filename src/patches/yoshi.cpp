#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "module.hxx"
#include "player.hxx"
#include "stage.hxx"

#include "p_settings.hxx"
#include <Camera/CubeMapTool.hxx>
#include <Camera/CubeManagerBase.hxx>
#include <Map/Map.hxx>

#ifndef ENABLE_WATER_CAVE_CAMERA_TYPE
#define ENABLE_WATER_CAVE_CAMERA_TYPE 0x100
#endif

using namespace BetterSMS;

static bool isYoshiMaintainFluddModel() {
    TMario *player;
    SMS_FROM_GPR(31, player);

    auto playerData = Player::getData(player);

    if (!playerData)
        return player->mAttributes.mHasFludd;

    if (player->mYoshi->mState == TYoshi::MOUNTED)
        return (playerData->mFluddHistory.mHadFludd && player->mAttributes.mHasFludd);
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
                 static_cast<f32>(player->mFludd->mNozzleList[playerData->mFluddHistory.mMainNozzle]
                                      ->mEmitParams.mAmountMax.get());
    changeNozzle__9TWaterGunFQ29TWaterGun11TNozzleTypeb(player->mFludd,
                                                        playerData->mFluddHistory.mSecondNozzle, 1);
    // player->normalizeNozzle();
    player->mFludd->mCurrentWater = player->mFludd->mNozzleList[(u8)player->mFludd->mCurrentNozzle]
                                        ->mEmitParams.mAmountMax.get() *
                                    factor;
    player->mAttributes.mHasFludd = playerData->mFluddHistory.mHadFludd;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024EC18, 0x802469A4, 0, 0), restoreNozzles);
SMS_WRITE_32(SMS_PORT_REGION(0x8024EC2C, 0x802469A8, 0, 0), 0x60000000);

static void killYoshi(TYoshi *yoshi) {
    if (gpMSound->gateCheck(31000))
        MSoundSE::startSoundActor(31000, yoshi->mTranslation, 0, nullptr, 0, 4);

    if (yoshi->mState == TYoshi::MOUNTED)
        yoshi->mMario->getOffYoshi(true);

    yoshi->mState = TYoshi::DROWNING;

    MActor *actor = yoshi->mActor;
    if (actor->getCurAnmIdx(MActor::BCK) != 25) {
        if (!actor->checkCurBckFromIndex(25))
            actor->setBckFromIndex(25);
        thinkBtp__6TYoshiFi(yoshi, 25);
        initAnmSound__9MAnmSoundFPvUlf(((u32 *)yoshi)[0x118 / 4],
                                       ((u32 **)yoshi)[0x11C / 4][0x64 / 4], 1, 0.0f);
    }

    yoshi->mType     = 0;
    yoshi->mSubState = 30;
}

extern f32 enhanceWaterCheckGeneric_(f32 x, f32 y, f32 z, bool considerCave,
                                                   const TMap *map, const TBGCheckData **water);

static void checkForWaterDeath(TYoshi *yoshi, const TBGCheckData *ground, f32 groundY) {
    if (yoshi->mType == TYoshi::GREEN || !BetterSMS::isCollisionRepaired())
        return;

    //// Check for water hit
    //if (ground->isWaterSurface())
    //    return;

    bool considerCave = false;

    TNameRefPtrAryT<TCubeCameraInfo> *cameraCubes = gpCubeCamera->getCubeInfo<TCubeCameraInfo>();

    if (cameraCubes) {
        for (size_t i = 0; i < cameraCubes->mChildren.size(); ++i) {
            if (!gpCubeCamera->isInCube(
                    {yoshi->mTranslation.x, yoshi->mTranslation.y, yoshi->mTranslation.z}, i)) {
                continue;
            }

            if (gpCubeCamera->getDataNo(i) == ENABLE_WATER_CAVE_CAMERA_TYPE) {
                considerCave = true;
                break;
            }
        }
    }

    const TBGCheckData *water = nullptr;
    f32 height = enhanceWaterCheckGeneric_(yoshi->mTranslation.x, yoshi->mTranslation.y + 10.0f,
                                           yoshi->mTranslation.z,
                                           considerCave, gpMap, &water);

    if (height - yoshi->mTranslation.y < 100)
        return;

    if (!water || !water->isWaterSurface() ||
        !(yoshi->mState == TYoshi::UNMOUNTED || yoshi->mState == TYoshi::MOUNTED))
        return;

    killYoshi(yoshi);
}

static void checkForOOBDeath(TYoshi *yoshi, const TBGCheckData *ground, f32 groundY) {
    if (!BetterSMS::isCollisionRepaired())
        return;

    if (yoshi->mState != TYoshi::UNMOUNTED)
        return;

    if (ground->mType != 1536 && ground->mType != 2048)
        return;

    if (yoshi->mTranslation.y - groundY > 200)
        return;

    killYoshi(yoshi);
}

BETTER_SMS_FOR_CALLBACK void checkForYoshiDeath(TMario *player, bool isMario) {
    if (!player->mYoshi)
        return;

    // Check for water death
    const TBGCheckData *ground;
    f32 groundY = gpMapCollisionData->checkGround(player->mYoshi->mTranslation.x,
                                                  player->mYoshi->mTranslation.y,
                                                  player->mYoshi->mTranslation.z, 0, &ground);

    checkForWaterDeath(player->mYoshi, ground, groundY);
    checkForOOBDeath(player->mYoshi, ground, groundY);
}

BETTER_SMS_FOR_CALLBACK void forceValidRidingAnimation(TMario *player, bool isMario) {
    TYoshi *yoshi = player->mYoshi;
    if (!yoshi)
        return;

    bool isMarioGrounded =
        !(player->mState & TMario::STATE_AIRBORN) && !(player->mState & TMario::STATE_WATERBORN);

    // Force valid animation
    if (yoshi->mState == TYoshi::MOUNTED && isMarioGrounded && BetterSMS::areExploitsPatched())
        player->setAnimation(TMario::ANIMATION_IDLE, 1.0f);
}

static bool isFixShallowWaterBug(TBGCheckData *data) {
    return data->isWaterSurface() && !BetterSMS::areBugsPatched();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8026E9D4, 0, 0, 0),
             isFixShallowWaterBug);  // Fix shallow water flashing
SMS_PATCH_BL(SMS_PORT_REGION(0x8026F144, 0, 0, 0), isFixShallowWaterBug);

static void checkYoshiFruitFix() {
    TYoshi *yoshi;
    SMS_FROM_GPR(30, yoshi);

    if (BetterSMS::areExploitsPatched())
        ((u16 **)(yoshi))[0x38 / 4][0x7C / 2] = 3;
    else
        ((u16 **)(yoshi))[0x38 / 4][0x7C / 2] = 0;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802703CC, 0, 0, 0), checkYoshiFruitFix);  // Fix fruit storage

static bool checkShouldMount() {
    TMario *player;
    SMS_FROM_GPR(31, player);

    return player->mSpeed.y < (BetterSMS::areExploitsPatched() ? -1.0f : 0.0f);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8028113C, 0x80278EC8, 0, 0), checkShouldMount);
SMS_WRITE_32(SMS_PORT_REGION(0x80281140, 0x80278ECC, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x80281144, 0x80278ED0, 0, 0), 0x41820134);

static void keepDistanceIsolated(TMario *player, f32 x, f32 y) {
    player->keepDistance(player->mYoshi->mTranslation, x, y);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80281284, 0, 0, 0), keepDistanceIsolated);

static void checkForYoshiGrabFludd(TWaterGun *fludd, int nozzle, bool unk_1) {
    fludd->changeNozzle(
        (TWaterGun::TNozzleType)(fludd->mMario->onYoshi() ? TWaterGun::Yoshi : nozzle), unk_1);

    auto playerData = Player::getData(fludd->mMario);
    if (!playerData->isMario())
        return;

    playerData->mFluddHistory.mHadFludd = true;
    playerData->mFluddHistory.mWaterLevel =
        fludd->mNozzleList[fludd->mCurrentNozzle]->mEmitParams.mAmountMax.get();
}
SMS_WRITE_32(SMS_PORT_REGION(0x80283334, 0, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x80283370, 0, 0, 0), checkForYoshiGrabFludd);

// Important to keep this patch here in order to stabilize things
static f32 getYoshiYPos(TYoshi *yoshi) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    return player->mYoshi->mTranslation.y;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80281148, 0x80278ED4, 0, 0), getYoshiYPos);