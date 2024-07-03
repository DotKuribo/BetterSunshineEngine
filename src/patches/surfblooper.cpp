#include "module.hxx"

BETTER_SMS_FOR_CALLBACK void onPlayerSurfingUpdate(TMario *player, bool isMario) {
    if (!isMario || !BetterSMS::areBugsPatched()) {
        return;
    }

    if (player->mState != 0x810446 && player->mState != 0x281089A) {
        return;
    }

    if ((player->mController->mButtons.mFrameInput & TMarioGamePad::X)) {
        player->mSurfGesso = nullptr;
        player->mSurfGessoID = 0;
        player->changePlayerStatus(TMario::STATE_BACK_FLIP, 0, false);
        return;
    }
}