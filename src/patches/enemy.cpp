#include <SMS/Enemy/Conductor.hxx>
#include <SMS/Enemy/SpineEnemy.hxx>

#include "module.hxx"

static void killEnemyWithinPatch() {
    TLiveActor *actor;
    SMS_FROM_GPR(28, actor);

    actor->kill();
    actor->mStateFlags.asU32 |= 0xD9;
    actor->mObjectType |= 1;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8003DEAC, 0, 0, 0), killEnemyWithinPatch);
SMS_WRITE_32(SMS_PORT_REGION(0x8003DEB0, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8003DEB4, 0, 0, 0), 0x60000000);