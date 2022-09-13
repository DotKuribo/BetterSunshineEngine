#include <SMS/macros.h>

#include "module.hxx"

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