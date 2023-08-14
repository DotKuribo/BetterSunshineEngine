#include <Dolphin/MTX.h>
#include <Dolphin/math.h>
#include <Dolphin/types.h>

#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/macros.h>

#include "module.hxx"
#include "p_warp.hxx"
#include "player.hxx"

using namespace BetterSMS;

static SMS_ASM_FUNC void checkGrabHeight() {
    SMS_ASM_BLOCK("lfs 0, " SMS_STRINGIZE(
        SMS_PORT_REGION(-0xED4, -0x105c, 0, 0)) "(2)\n\t"
                                                "lfs 4, 0x28(29)                     \n\t"
                                                "fmuls 0, 0, 4                       \n\t"
                                                "blr                                 \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80256D34, 0x8024EAC0, 0, 0), checkGrabHeight);

static SMS_ASM_FUNC void setCollisionHeight1() {
    SMS_ASM_BLOCK("lfs 1, " SMS_STRINGIZE(
        SMS_PORT_REGION(-0xEDC, -0x1064, 0, 0)) "(2)                  \n\t"
                                                "lfs 0, 0x28(22)            \n\t"
                                                "fmuls 1, 0, 1              \n\t"
                                                "blr                        \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8025696C, 0x8024E6F8, 0, 0), setCollisionHeight1);

static SMS_ASM_FUNC void setCollisionHeight2() {
    SMS_ASM_BLOCK("lfs 2, " SMS_STRINGIZE(
        SMS_PORT_REGION(-0xEDC, -0x1064, 0, 0)) "(2)                        \n\t"
                                                "lfs 0, 0x28(29)            \n\t"
                                                "fmuls 2, 0, 2              \n\t"
                                                "blr                        \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80256D14, 0x8024EAA0, 0, 0), setCollisionHeight2);

static SMS_ASM_FUNC void setCollisionHeight3() {
    SMS_ASM_BLOCK("lfs 0, " SMS_STRINGIZE(
        SMS_PORT_REGION(-0xEDC, -0x1064, 0, 0)) "(2)                        \n\t"
                                                "lfs 2, 0x28(30)            \n\t"
                                                "fmuls 0, 2, 0              \n\t"
                                                "blr                        \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802573FC, 0x8024F188, 0, 0), setCollisionHeight3);

static void setCollisionWidth() {
    TMario *player;
    SMS_FROM_GPR(29, player);

    f32 width = 50.0f;

    Vec size;
    player->JSGGetScaling(&size);

    auto playerData = Player::getData(player);
    if (playerData->isMario())
        width *= size.x;

    player->mCollisionXZSize = width;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802505F4, 0x80248380, 0, 0), setCollisionWidth);

static f32 manageGrabLength() {
    TMario *player;
    SMS_FROM_GPR(29, player);

    Vec size;
    player->JSGGetScaling(&size);

    return 60.0f * size.z;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80256CE8, 0x8024EA74, 0, 0), manageGrabLength);
SMS_WRITE_32(SMS_PORT_REGION(0x80256CFC, 0x8024EA88, 0, 0), 0xEC01283C);
SMS_WRITE_32(SMS_PORT_REGION(0x80256D04, 0x8024EA90, 0, 0), 0xC05E003C);
SMS_WRITE_32(SMS_PORT_REGION(0x80256D0C, 0x8024EA98, 0, 0), 0xEC0100BC);

static f32 setBounceYSpeed() {
    TMario *player;
    SMS_FROM_GPR(30, player);

    Vec size;
    player->JSGGetScaling(&size);

    return 130.0f * size.y;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80254720, 0x8024C4AC, 0, 0), setBounceYSpeed);
SMS_WRITE_32(SMS_PORT_REGION(0x80254724, 0x8024C4B0, 0, 0), 0xD01E00A8);