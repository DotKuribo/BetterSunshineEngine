#include <Dolphin/MTX.h>
#include <JSystem/JGeometry/JGMVec.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>

#include "player.hxx"
#include "module.hxx"

#if BETTER_SMS_EXTRA_COLLISION

/* extern to handlers.cpp */

void setGravityCol(TMario *player, const TBGCheckData *data, u32 flags) {

    player->mJumpParams.mGravity.set(static_cast<f32>(data->mValue) / 100.0f);
}

void antiGravityCol(TMario *player, const TBGCheckData *data, u32 flags) {
    Vec position;
    player->JSGGetTranslation(&position);

    if ((position.y - data->mMaxHeight) > data->mValue && data->mValue != 0)
        return;

    player->mSpeed.y = 10.0f;
    if ((player->mState & static_cast<u32>(TMario::STATE_AIRBORN)) == false) {
        position.y += 1.0f;
        player->JSGSetTranslation(position);
        player->changePlayerStatus(TMario::STATE_FALL, 0, 0);
    }
    if (player->mState == static_cast<u32>(TMario::STATE_FALL))
        player->mSubStateTimer = 0;
}

#endif