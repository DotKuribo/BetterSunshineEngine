#include <Dolphin/types.h>

#include <SMS/macros.h>
#include <SMS/Player/Mario.hxx>

#include "module.hxx"

static u8 patchYStorageWalkEnd(TMario *mario) {
    mario->mSpeed.y = 0;
	return mario->walkEnd();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8025875C, 0, 0, 0), patchYStorageWalkEnd);

static void patchYStorageIdle(TMario *mario, f32 velocity) {
    mario->mSpeed.y = 0;
    mario->setPlayerVelocity(velocity);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8025719C, 0, 0, 0), patchYStorageIdle);