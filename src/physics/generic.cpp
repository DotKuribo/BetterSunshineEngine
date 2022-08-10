#include <Dolphin/MTX.h>
#include <Dolphin/math.h>
#include <Dolphin/types.h>

#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/actor/Mario.hxx>
#include <SMS/macros.h>
#include <SMS/npc/BaseNPC.hxx>


#include "collision/warp.hxx"
#include "common_sdk.h"
#include "geometry.hxx"
#include "module.hxx"
#include "player.hxx"

using namespace BetterSMS;

static u32 updateContexts(TMario *player) {
    extern void updateCollisionContext(TMario * player);
    extern void updateClimbContext(TMario * player);

    updateCollisionContext(player);
    updateClimbContext(player);

    return 1;
}

static void addVelocity(TMario *player, f32 velocity) {
    auto playerData = Player::getData(player);

    if (!onYoshi__6TMarioCFv(player)) {
        player->mForwardSpeed =
            Min(player->mForwardSpeed + velocity,
                (playerData->mMaxAddVelocity * playerData->getParams()->mSpeedMultiplier.get()) *
                    playerData->mSlideSpeedMultiplier);
    } else {
        player->mForwardSpeed =
            Min(player->mForwardSpeed + velocity,
                playerData->mMaxAddVelocity * playerData->mSlideSpeedMultiplier);
    }
}
SMS_PATCH_B(SMS_PORT_REGION(0x802558A4, 0x8024D630, 0, 0), addVelocity);

static void rescaleHeldObj(Mtx holderMatrix, Mtx destMatrix) {
    TMapObjBase *heldObj;
    SMS_FROM_GPR(31, heldObj);

    Vec holderSize;
    heldObj->mHolder->JSGGetScaling(&holderSize);

    PSMTXCopy(holderMatrix, destMatrix);
    PSMTXScaleApply(destMatrix, destMatrix, 1 / holderSize.x, 1 / holderSize.y, 1 / holderSize.z);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801E4118, 0x801DBFF0, 0, 0), rescaleHeldObj);