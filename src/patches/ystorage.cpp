#include <Dolphin/types.h>

#include <SMS/Player/Mario.hxx>
#include <SMS/macros.h>

#include "module.hxx"
#include "p_settings.hxx"

static u8 patchYStorageWalkEnd(TMario *mario) {
    if (BetterSMS::areExploitsPatched())
        mario->mSpeed.y = 0;
    return mario->walkEnd();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8025875C, 0, 0, 0), patchYStorageWalkEnd);

static void patchYStorageIdle(TMario *mario, f32 velocity) {
    if (BetterSMS::areExploitsPatched())
        mario->mSpeed.y = 0;
    mario->setPlayerVelocity(velocity);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8025719C, 0, 0, 0), patchYStorageIdle);