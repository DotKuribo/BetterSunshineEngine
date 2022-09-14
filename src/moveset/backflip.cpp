#include <SMS/SMS.hxx>
#include <SMS/actor/Mario.hxx>
#include <SMS/nozzle/Watergun.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/sound/MSound.hxx>
#include <SMS/sound/MSoundSESystem.hxx>

#include "module.hxx"
#include "player.hxx"

using namespace BetterSMS;

u32 CrouchState = 0xF00001C0;

SMS_WRITE_32(SMS_PORT_REGION(0x802A8860, 0, 0, 0), 0x540006F7);
SMS_WRITE_32(SMS_PORT_REGION(0x802A886C, 0, 0, 0), 0x540006F7);
SMS_WRITE_32(SMS_PORT_REGION(0x802A88C8, 0, 0, 0), 0x540006F7);
SMS_WRITE_32(SMS_PORT_REGION(0x802A88D4, 0, 0, 0), 0x540006F7);

void checkForCrouch(TMario *player, bool isMario) {
    if (PSVECMag(player->mSpeed) > __FLT_EPSILON__)
        return;

    if (player->mState & TMario::STATE_AIRBORN)
        return;

    if (player->mState & TMario::STATE_WATERBORN)
        return;

    auto *controller = player->mController;
    if (!controller)
        return;

    if (!(controller->mButtons.mInput & TMarioGamePad::L))
        return;

    changePlayerStatus__6TMarioFUlUlb(player, CrouchState, 0, false);
}

bool processCrouch(TMario *player) {
    auto *controller = player->mController;
    if (!controller) {
        changePlayerStatus__6TMarioFUlUlb(player, TMario::STATE_SIDE_STEP_LEAVE, 0, false);
        return true;
    }

    if (!(controller->mButtons.mInput & TMarioGamePad::L)) {
        changePlayerStatus__6TMarioFUlUlb(player, TMario::STATE_SIDE_STEP_LEAVE, 0, false);
        return true;
    }

    if (controller->mButtons.mInput & TMarioGamePad::A) {
        changePlayerStatus__6TMarioFUlUlb(player, TMario::STATE_BACK_FLIP, 0, false);
        return true;
    }

    setAnimation__6TMarioFif(player, TMario::ANIMATION_STEADY_STANCE);
    return false;
}