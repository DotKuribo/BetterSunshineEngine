#include <Dolphin/mem.h>
#include <JSystem/JGeometry.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>

#include <SMS/game/Application.hxx>
#include <SMS/manager/MarioParticleManager.hxx>
#include <SMS/screen/SMSFader.hxx>
#include <SMS/sound/MSoundSESystem.hxx>

#include "collision/warp.hxx"
#include "globals.hxx"
#include "libs/triangle.hxx"
#include "logging.hxx"
#include "player.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Geometry;
using namespace BetterSMS::Collision;

void decHealth(TMario *player, const TBGCheckData *data, u32 flags) {
    auto *playerData = Player::getData(player);
    if (playerData->mCollisionTimer <= 0) {
        player->decHP(1);
        playerData->mCollisionTimer = player->mFloorTriangle->mValue4;
    } else
        playerData->mCollisionTimer -= 1;
}

void incHealth(TMario *player, const TBGCheckData *data, u32 flags) {
    auto *playerData = Player::getData(player);
    if (playerData->mCollisionTimer <= 0) {
        player->incHP(1);
        playerData->mCollisionTimer = player->mFloorTriangle->mValue4;
    } else
        playerData->mCollisionTimer -= 1;
}