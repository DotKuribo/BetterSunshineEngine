#include <SMS/macros.h>

#include "player.hxx"
#include "raw_fn.hxx"
#include "module.hxx"

/* NPC CARRY CODE */

// 0x8029A87C
static u32 carryOrTalkNPC(TBaseNPC *npc) {
    const Player::TPlayerData *playerData = Player::getData(gpMarioAddress);

    if (!playerData->isMario())
        return isNowCanTaken__8TBaseNPCCFv(npc);

    if ((*(u32 *)(&npc->mStateFlags.asFlags) & 0x840007) != 0)
        return 0;

    if (gpMarioAddress->mState == static_cast<u32>(TMario::STATE_IDLE))
        return 0;

    bool oldTake                         = npc->mStateFlags.asFlags.mCanBeTaken;
    npc->mStateFlags.asFlags.mCanBeTaken = playerData->getParams()->mCanHoldNPCs.get();

    u32 ret = isNowCanTaken__8TBaseNPCCFv(npc);

    npc->mStateFlags.asFlags.mCanBeTaken = oldTake;
    return ret;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8029A87C, 0x80292758, 0, 0), carryOrTalkNPC);

// 0x802815F0
static bool canGrabAtNPC() {
    TBaseNPC *npc;
    SMS_FROM_GPR(30, npc);

    const Player::TPlayerData *playerData = Player::getData(gpMarioAddress);

    if (!playerData->isMario())
        return npc->mStateFlags.asFlags.mCanBeTaken;

    return (playerData->getParams()->mCanHoldNPCs.get() &&
            gpMarioAddress->mState != static_cast<u32>(TMario::STATE_IDLE)) ||
           npc->mStateFlags.asFlags.mCanBeTaken;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802815F0, 0x8027937C, 0, 0), canGrabAtNPC);
SMS_WRITE_32(SMS_PORT_REGION(0x802815F4, 0x80279380, 0, 0), 0x2C030000);

// 0x80207430
static bool canCarryNPC() {
    TBaseNPC *npc;
    SMS_FROM_GPR(29, npc);

    const Player::TPlayerData *playerData = Player::getData(gpMarioAddress);

    if (!playerData->isMario())
        return npc->mStateFlags.asFlags.mCanBeTaken;

    return (playerData->getParams()->mCanHoldNPCs.get() &&
            gpMarioAddress->mState != static_cast<u32>(TMario::STATE_IDLE)) ||
           npc->mStateFlags.asFlags.mCanBeTaken;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80207430, 0x801FF314, 0, 0), canCarryNPC);
SMS_WRITE_32(SMS_PORT_REGION(0x80207434, 0x801FF318, 0, 0), 0x2C030000);

// extern -> SME.cpp
// 0x8021463C
static u32 scaleNPCThrow() {
  TBaseNPC *npc;
  SMS_FROM_GPR(31, npc);

  TMario *player                        = (TMario *)npc->mPrevHolder;
  const Player::TPlayerData *playerData = Player::getData(gpMarioAddress);

  if (playerData->isMario()) {
      npc->mSpeed.scale(playerData->getParams()->mThrowPowerMultiplier.get());
      if (player->mState == TMario::STATE_NPC_PUTDOWN) {
          npc->mSpeed = {0.0f, 0.0f, 0.0f};
      }
  }

  //if (player->mState == static_cast<u32>(TMario::STATE_NPC_THROW) ||
  //    player->mState == static_cast<u32>(TMario::STATE_NPC_JUMPTHROW))
  //  y *= 4.0f;


  return CLBPalFrame_l(15);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80214650, 0x8020C534, 0, 0), scaleNPCThrow);

// 0x80213314
static SMS_ASM_FUNC void scaleNPCTalkRadius() {
    SMS_ASM_BLOCK("lis 3, gpMarioAddress@ha                \n\t"
                  "lwz 3, gpMarioAddress@l (3)             \n\t"
                  "lfs 0, 0x2C (3)                         \n\t"
                  "fmuls 30, 30, 0                         \n\t"
                  "lis 3, mPtrSaveNormal__8TBaseNPC@ha     \n\t"
                  "lwz 3, mPtrSaveNormal__8TBaseNPC@l (3)  \n\t"
                  "blr                                     \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80213314, 0x8020B1FC, 0, 0), scaleNPCTalkRadius);