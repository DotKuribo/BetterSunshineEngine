#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>


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
    player->normalizeNozzle();
    player->mFludd->mCurrentWater = player->mFludd->mNozzleList[(u8)player->mFludd->mCurrentNozzle]
                                        ->mEmitParams.mAmountMax.get() *
                                    factor;
    player->mAttributes.mHasFludd = playerData->mFluddHistory.mHadFludd;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8024EC18, 0x802469A4, 0, 0), restoreNozzles);
SMS_WRITE_32(SMS_PORT_REGION(0x8024EC2C, 0x802469A8, 0, 0), 0x60000000);

#endif

#if BETTER_SMS_GREEN_YOSHI

static void checkForWaterDeath(TYoshi *yoshi, const TBGCheckData *ground, f32 groundY) {
    if (yoshi->mType == TYoshi::GREEN)
        return;

    // Check for water hit
    if (ground->isWaterSurface())
        return;

    const TBGCheckData *roof;
    f32 roofHeight = gpMapCollisionData->checkRoof(yoshi->mPosition.x, yoshi->mPosition.y,
                                                   yoshi->mPosition.z, 0, &roof);
    const TBGCheckData *water;
    f32 waterY = gpMapCollisionData->checkGround(yoshi->mPosition.x, roofHeight - 10.0f, yoshi->mPosition.z, 0,
                                                 &water);

    if (waterY - yoshi->mPosition.y < 100)
        return;

    if (!water->isWaterSurface() ||
        !(yoshi->mState == TYoshi::UNMOUNTED || yoshi->mState == TYoshi::MOUNTED))
        return;

    if (gpMSound->gateCheck(31000))
        MSoundSE::startSoundActor(31000, yoshi->mPosition, 0, nullptr, 0, 4);

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

static void checkForOOBDeath(TYoshi *yoshi, const TBGCheckData *ground, f32 groundY) {
    if (yoshi->mState != TYoshi::UNMOUNTED)
        return;

    if (ground->mType != 1536 && ground->mType != 2048)
        return;

    if (yoshi->mPosition.y - groundY > 200)
        return;

    if (gpMSound->gateCheck(31000))
        MSoundSE::startSoundActor(31000, yoshi->mPosition, 0, nullptr, 0, 4);

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

void checkForYoshiDeath(TMario *player, bool isMario) {
    if (!player->mYoshi)
        return;

    // Check for water death
    const TBGCheckData *ground;
    f32 groundY = gpMapCollisionData->checkGround(player->mYoshi->mPosition.x, player->mYoshi->mPosition.y,
                                    player->mYoshi->mPosition.z, 0,
                                    &ground);

    checkForWaterDeath(player->mYoshi, ground, groundY);
    checkForOOBDeath(player->mYoshi, ground, groundY);
}

void forceValidRidingAnimation(TMario *player, bool isMario) {
    TYoshi *yoshi = player->mYoshi;
    if (!yoshi)
        return;

    // Force valid animation
    if (yoshi->mState == TYoshi::MOUNTED && player->mState != TMario::STATE_SHINE_C)
        player->setAnimation(TMario::ANIMATION_IDLE, 1.0f);
}

SMS_WRITE_32(SMS_PORT_REGION(0x8026E9DC, 0, 0, 0), 0x60000000);  // Fix shallow water flashing
SMS_WRITE_32(SMS_PORT_REGION(0x8026F14C, 0, 0, 0), 0x60000000);

SMS_WRITE_32(SMS_PORT_REGION(0x802703C8, 0, 0, 0), 0x38000003);  // Fix fruit storage

static bool checkShouldMount() {
    TMario *player;
    SMS_FROM_GPR(31, player);

    return player->mSpeed.y < -1.0f;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8028113C, 0x80278EC8, 0, 0), checkShouldMount);
SMS_WRITE_32(SMS_PORT_REGION(0x80281140, 0x80278ECC, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x80281144, 0x80278ED0, 0, 0), 0x41820134);

static void keepDistanceIsolated(TMario *player, f32 x, f32 y) {
    player->keepDistance(player->mYoshi->mPosition, x, y);
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

#endif