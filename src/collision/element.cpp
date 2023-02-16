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
#include "p_warp.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Collision;

// types start at 3000

BETTER_SMS_FOR_CALLBACK void elecPlayer(TMario *player, const TBGCheckData *data, u32 flags) {
    if (!(flags & Player::InteractionFlags::GROUNDED))
        return;

    if (!player->isInvincible()) {
        player->decHP(data->mValue);
        player->changePlayerStatus(0x20338, 0, false);
        player->mInvincibilityFrames = 256;

        if (gpMSound->gateCheck(0x1814)) {
            MSoundSE::startSoundActor(0x1814, player->mTranslation, 0, nullptr, 0, 4);
        }
        if (gpMSound->gateCheck(0x3806)) {
            MSoundSE::startSoundActor(0x3806, player->mTranslation, 0, nullptr, 0, 4);
        }
    }
}

BETTER_SMS_FOR_CALLBACK void burnPlayer(TMario *player, const TBGCheckData *data, u32 flags) {
    if (!(flags & Player::InteractionFlags::GROUNDED))
        return;

    if (!player->isInvincible()) {
        player->changePlayerDropping(0x20464, 0);
        player->decHP(data->mValue);
        player->dropObject();
        player->changePlayerStatus(0x208B7, 1, false);
        player->mInvincibilityFrames = 256;
        player->mSpeed.y += 20.0f;

        gpMarioParticleManager->emitAndBindToPosPtr(6, &player->mTranslation, 0, nullptr);
        if (gpMSound->gateCheck(0x1813)) {
            MSoundSE::startSoundActor(0x1813, player->mTranslation, 0, nullptr, 0, 4);
        }
    }
}