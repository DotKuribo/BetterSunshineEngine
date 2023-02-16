#include <Dolphin/mem.h>
#include <JSystem/JGeometry/JGMVec.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>

#include <SMS/System/Application.hxx>
#include <SMS/Manager/MarioParticleManager.hxx>
#include <SMS/GC2D/SMSFader.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>

#include "libs/triangle.hxx"
#include "logging.hxx"
#include "player.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Collision;

BETTER_SMS_FOR_CALLBACK void decHealth(TMario *player, const TBGCheckData *data, u32 flags) {
    auto *playerData = Player::getData(player);
    if (playerData->mCollisionTimer <= 0) {
        player->decHP(1);
        playerData->mCollisionTimer = player->mFloorTriangle->mValue;
    } else
        playerData->mCollisionTimer -= 1;
}

BETTER_SMS_FOR_CALLBACK void incHealth(TMario *player, const TBGCheckData *data, u32 flags) {
    auto *playerData = Player::getData(player);
    if (playerData->mCollisionTimer <= 0) {
        player->incHP(1);
        playerData->mCollisionTimer = player->mFloorTriangle->mValue;
    } else
        playerData->mCollisionTimer -= 1;
}