#include <SMS/Player/Mario.hxx>

#include <SMS/game/GCConsole2.hxx>
#include <SMS/nozzle/Watergun.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/sound/MSound.hxx>
#include <SMS/sound/MSoundSESystem.hxx>

#include "module.hxx"
#include "player.hxx"

using namespace BetterSMS;

#if BETTER_SMS_HOVER_SLIDE

// static bool isPumpOK(TMarioAnimeData *animeData) {
//     const bool defaultEnabled = animeData->mFluddEnabled !=
//     TMarioAnimeData::FLUDD::FLUDD_DISABLED;

//     auto *playerData = Player::getData(gpMarioAddress);
//     if (!playerData)
//         return defaultEnabled;

//     return defaultEnabled && playerData->mCurJump <= 1;
// }
// SMS_PATCH_B(SMS_PORT_REGION(0x80248F14, 0x80240CA0, 0, 0), isPumpOK);

SMS_WRITE_32(SMS_PORT_REGION(0x803DCA00, 0x803D41E0, 0, 0),  // Allow dive spray
             0x00300000 | TMarioAnimeData::FLUDD::FLUDD_ENABLED);
#endif

// static TWaterGun *bindFluddtojoint() {
//     TMario *player;
//     SMS_FROM_GPR(31, player);

//     auto *playerData = Player::getData(player);
//     if (!playerData)
//         return player->mFludd;

//     player->mBindBoneIDArray[0] = playerData->getNozzleBoneID(
//         static_cast<TWaterGun::TNozzleType>(player->mFludd->mCurrentNozzle));

//     return player->mFludd;
// }
// SMS_PATCH_BL(SMS_PORT_REGION(0x8024D53C, 0x802452C8, 0, 0), bindFluddtojoint);

static bool hasWaterCardOpen() {
    TGCConsole2 *gcConsole;
    SMS_FROM_GPR(31, gcConsole);

    if (gpMarioAddress->mYoshi->mState != TYoshi::State::MOUNTED &&
        !gpMarioAddress->mAttributes.mHasFludd && !gcConsole->mWaterCardFalling &&
        gcConsole->mIsWaterCard)
        startDisappearTank__11TGCConsole2Fv(gcConsole);
    else if (gpMarioAddress->mYoshi->mState == TYoshi::State::MOUNTED)
        gpMarioAddress->mAttributes.mHasFludd = true;

    return gcConsole->mIsWaterCard;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8014206C, 0x80136C80, 0, 0), hasWaterCardOpen);
SMS_WRITE_32(SMS_PORT_REGION(0x80142070, 0x80136C84, 0, 0), 0x28030000);

static bool canCollectFluddItem(TMario *player) {
    const bool defaultEnabled = player->onYoshi();

    auto *playerData = Player::getData(gpMarioAddress);
    if (!playerData)
        return defaultEnabled;

    return defaultEnabled || !playerData->getCanUseFludd();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80283058, 0x8027ADE4, 0, 0), canCollectFluddItem);

static s32 sNozzleBuzzCounter = -1;
static bool canCollectFluddItem_() {
    TMario *player;
    SMS_FROM_GPR(30, player);

    const bool isOnYoshi = player->onYoshi();

    auto *playerData = Player::getData(gpMarioAddress);
    if (!playerData)
        return isOnYoshi;

    if (!playerData->getCanUseFludd()) {
        if (gpMSound->gateCheck(0x483E) && sNozzleBuzzCounter < 0) {
            MSoundSESystem::MSoundSE::startSoundSystemSE(0x483E, 0, nullptr, 0);
            sNozzleBuzzCounter = 120;
        } else {
            sNozzleBuzzCounter -= 1;
        }
    }
    return isOnYoshi || !playerData->getCanUseFludd();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BBD48, 0x801B3C00, 0, 0), canCollectFluddItem_);

static void resetNozzleBuzzer(TMapObjGeneral *obj) {
    if (obj->mNumObjs <= 0) {
        sNozzleBuzzCounter = Max(sNozzleBuzzCounter - 1, -1);
    }
    control__14TMapObjGeneralFv(obj);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BBBF8, 0x801B3AB0, 0, 0), resetNozzleBuzzer);

#if BETTER_SMS_ROCKET_DIVE
static void checkRocketNozzleDiveBlast(TNozzleTrigger *nozzle, u32 r4, TWaterEmitInfo *emitInfo) {
    TMario *player = nozzle->mFludd->mMario;

    if (nozzle->mFludd->mCurrentNozzle != TWaterGun::Rocket)
        return;

    nozzle->mEmitParams.mReactionPow.set(player->mState != TMario::STATE_DIVE ? 0.0f : 0.5f);
}
#else
static void checkRocketNozzleDiveBlast(TNozzleTrigger *nozzle, u32 r4, TWaterEmitInfo *emitInfo) {}
#endif

// Fludd mods hook
void fluddEmitModWrapper(TNozzleTrigger *nozzle, u32 r4, TWaterEmitInfo *emitInfo) {
    void (*virtualFunc)(TNozzleTrigger *, u32, TWaterEmitInfo *);
    SMS_FROM_GPR(12, virtualFunc);

    /* CODE HERE */

    extern void checkSpamHover(TNozzleTrigger * nozzle, u32 r4, TWaterEmitInfo * emitInfo);
    checkSpamHover(nozzle, r4, emitInfo);
    checkRocketNozzleDiveBlast(nozzle, r4, emitInfo);

    virtualFunc(nozzle, r4, emitInfo);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8026C018, 0x80263DA4, 0, 0), fluddEmitModWrapper);