#include <Dolphin/MTX.h>
#include <Dolphin/math.h>
#include <Dolphin/types.h>

#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/macros.h>

#include "module.hxx"
#include "player.hxx"

using namespace BetterSMS;

/* GOOP WALKING CODE */

// extern -> SME.cpp
// 0x8024E288
// static void checkGraffitiAffected(TMario *player) {
//    auto playerData = Player::getData(player);
//
//    if (!playerData->isMario()) {
//        player->checkGraffito();
//    } else if (playerData->getParams()->mGoopAffected.get()) {
//        player->checkGraffito();
//    }
//}
// SMS_PATCH_BL(SMS_PORT_REGION(0x8024E288, 0x80246014, 0, 0), checkGraffitiAffected);