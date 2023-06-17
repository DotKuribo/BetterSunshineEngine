#include <Dolphin/MTX.h>
#include <Dolphin/math.h>
#include <Dolphin/types.h>

#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/macros.h>
#include <SMS/MapObj/MapObjTree.hxx>
#include <SMS/NPC/NpcBase.hxx>


#include "libs/constmath.hxx"
#include "module.hxx"
#include "player.hxx"
#include "p_settings.hxx"

using namespace BetterSMS;

//static void climbSometimes(TMario *player) {
//    auto playerData = Player::getData(player);
//
//    const bool isMarioClimb = isMarioClimb__16TCameraMarioDataCFUl(player->mState);
//    if (playerData->mIsOnFire) {
//        if (isMarioClimb)
//            player->changePlayerStatus(TMario::STATE_FALL, 0, false);
//        return;
//    }
//    player->barProcess();
//}
// SMS_PATCH_BL(SMS_PORT_REGION(0x8025D354, 0, 0, 0), climbSometimes);

BETTER_SMS_FOR_CALLBACK void updateClimbContext(TMario *player, bool isMario) {
    if (!BetterSMS::areExploitsPatched()) {
        return;
    }

    if ((player->mState & 0x1C0) == 320) {
        if (player->mCeilingAbove >= 9999990.0f && (player->mState & 0x200000) != 0)
            player->mActionState |= 0x8000;  // patch upwarps
    }
}

/* PATCHES */

/* TREE CLIMB CODE */

// static void overrideTreeClimbFromFloor(TMario *player) {
//     const TBGCheckData *data;
//     f32 groundHeight = gpMapCollisionData->checkGround(player->mTranslation.x, player->mTranslation.y,
//                                                        player->mTranslation.z, 1, &data);

//     player->mHolderHeightDiff = Min(player->mHolderHeightDiff, player->mTranslation.y -
//     groundHeight);

//     player->treeSlipEffect();
// }
// SMS_PATCH_BL(SMS_PORT_REGION(0x80261D38, 0, 0, 0), overrideTreeClimbFromFloor);

// extern -> SME.cpp
// 0x80261C3C
static f32 getTreeWaitParameters() {
    TMario *player;
    SMS_FROM_GPR(31, player);

    const TBGCheckData *data;
    f32 groundHeight = gpMapCollisionData->checkGround(player->mTranslation.x, player->mTranslation.y,
                                                       player->mTranslation.z, 1, &data);

    const f32 padding = Max(player->mHolderHeightDiff - (player->mTranslation.y - groundHeight), 0);

    Vec size;
    player->JSGGetScaling(&size);

    return (100.0f * size.y) + padding;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80261C3C, 0x802599C8, 0, 0), getTreeWaitParameters);
SMS_WRITE_32(SMS_PORT_REGION(0x80261C40, 0x802599CC, 0, 0), 0xC05F038C);
SMS_WRITE_32(SMS_PORT_REGION(0x80261C44, 0x802599D0, 0, 0), 0xFC020840);

static bool getTreeClimbParameters(TMapObjTree *tree) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    const TBGCheckData *data;
    f32 groundHeight = gpMapCollisionData->checkGround(player->mTranslation.x, player->mTranslation.y,
                                                       player->mTranslation.z, 1, &data);

    const f32 receiveHeight = tree->mReceiveHeight;
    const f32 treeScaleY    = scaleLinearAtAnchor(tree->mScale.y, 0.2f, 0.0f);

    return player->mTranslation.y > tree->mTranslation.y + (receiveHeight + treeScaleY);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802619CC, 0, 0, 0), getTreeClimbParameters);
SMS_WRITE_32(SMS_PORT_REGION(0x802619D0, 0, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x802619D4, 0, 0, 0), 0x4182003C);
SMS_WRITE_32(SMS_PORT_REGION(0x802619D8, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x802619DC, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x802619E0, 0, 0, 0), 0x60000000);

// This function serves to make f1 safe for earlier processing! ^^
static void wrapVelocityReset(TMario *player) { player->setPlayerVelocity(0.0f); }
SMS_PATCH_BL(SMS_PORT_REGION(0x802619E8, 0, 0, 0), wrapVelocityReset);

// extern -> SME.cpp
// 0x80261CF4
static SMS_ASM_FUNC void scaleTreeSlideSpeed() {
    // F2 IS UNSAFE
    SMS_ASM_BLOCK("mflr 0                      \n\t"
                  "stw 0, 0x8 (1)              \n\t"
                  "stwu 1, -0x10 (1)           \n\t"
                  "lfs 3, 0x5C (3)             \n\t"
                  "bl _localdata_              \n\t"
                  ".float 0.00195313, -16      \n\t"
                  "_localdata_:                \n\t"
                  "mflr 11                     \n\t"
                  "lfs 0, 0 (11)               \n\t"
                  "lfs 1, 4 (11)               \n\t"
                  "lfs 4, 0xB18 (31)           \n\t"
                  "fmuls 4, 4, 0               \n\t"
                  "fcmpo 0, 2, 1               \n\t"
                  "li 3, 1                     \n\t"
                  "blt _exit666                \n\t"
                  "li 3, 0                     \n\t"
                  "stw 3, 0xA8 (31)            \n\t"
                  "_exit666:                   \n\t"
                  "addi 1, 1, 0x10             \n\t"
                  "lwz 0, 0x8 (1)              \n\t"
                  "mtlr 0                      \n\t"
                  "blr                         \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80261CF4, 0x80259A80, 0, 0), scaleTreeSlideSpeed);
SMS_WRITE_32(SMS_PORT_REGION(0x80261CF8, 0x80259A84, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x80261CFC, 0x80259A88, 0, 0), 0x41820070);

/* GLOBAL CLIMB CODE */

void getClimbingAnimSpd(TMario *player, TMario::Animation anim, f32 speed) {
    auto playerData = Player::getData(player);

    if (playerData->mIsClimbTired)
        speed = 6.0f;

    player->setAnimation(anim, speed);
}
// SMS_PATCH_BL(SMS_PORT_REGION(0x8025D588, 0, 0, 0), getClimbingAnimSpd);
// SMS_PATCH_BL(SMS_PORT_REGION(0x8025D63C, 0, 0, 0), getClimbingAnimSpd);
// SMS_PATCH_BL(SMS_PORT_REGION(0x8025D650, 0, 0, 0), getClimbingAnimSpd);
// SMS_PATCH_BL(SMS_PORT_REGION(0x8025DBC4, 0, 0, 0), getClimbingAnimSpd);
// SMS_PATCH_BL(SMS_PORT_REGION(0x8025E38C, 0, 0, 0), getClimbingAnimSpd);

/* ROOF HANG CODE */

static SMS_ASM_FUNC void scaleRoofClimbHeight(f32 yCoord, f32 speed) {
    SMS_ASM_BLOCK("lfs 0, " SMS_STRINGIZE(SMS_PORT_REGION(
        -0xDE0, -0xf68, 0, 0)) "(2)        \n\t"
                               "lfs 3, 0x28(31)                                \n\t"
                               "fmuls 0, 0, 3                                  \n\t"
                               "blr                                            \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8025D66C, 0x802553F8, 0, 0), scaleRoofClimbHeight);

static SMS_ASM_FUNC void scaleRoofSquashedHeight() {
    SMS_ASM_BLOCK("lfs 3, " SMS_STRINGIZE(SMS_PORT_REGION(
        -0xDE0, -0xf68, 0, 0)) "(2)        \n\t"
                               "lfs 5, 0x28(30)                                \n\t"
                               "fmuls 3, 5, 3                                  \n\t"
                               "blr                                            \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802617EC, 0x80259578, 0, 0), scaleRoofSquashedHeight);

static SMS_ASM_FUNC void scaleRoofMoveDiff() {
    SMS_ASM_BLOCK("lfs 0, " SMS_STRINGIZE(SMS_PORT_REGION(
        -0xD7C, -0xf04, 0, 0)) "(2)        \n\t"
                               "lfs 10, 0x28(30)                                \n\t"
                               "fmuls 0, 0, 10                                  \n\t"
                               "blr                                             \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80261824, 0x802595B0, 0, 0), scaleRoofMoveDiff);

//static bool canHangOnRoof(TBGCheckData *roof /*, u16 colType*/) {
//    TMario *player;
//    SMS_FROM_GPR(30, player);
//
//    u16 colType;
//    SMS_FROM_GPR(4, colType);
//
//    auto playerData = Player::getData(player);
//
//    if (playerData->isMario() && playerData->getParams()->mCanClimbWalls.get())
//        return true;
//
//    return colType == 266;
//}
//SMS_WRITE_32(SMS_PORT_REGION(0x802617C0, 0x8025954C, 0, 0), 0xA0830000);
//SMS_PATCH_BL(SMS_PORT_REGION(0x802617C4, 0x80259550, 0, 0), canHangOnRoof);
//SMS_WRITE_32(SMS_PORT_REGION(0x802617C8, 0x80259554, 0, 0), 0x60000000);
//SMS_WRITE_32(SMS_PORT_REGION(0x802617CC, 0x80259558, 0, 0), 0x60000000);
//SMS_WRITE_32(SMS_PORT_REGION(0x802617D0, 0x8025955C, 0, 0), 0x60000000);
//SMS_WRITE_32(SMS_PORT_REGION(0x802617D4, 0x80259560, 0, 0), 0x60000000);
//SMS_WRITE_32(SMS_PORT_REGION(0x802617D8, 0x80259564, 0, 0), 0x2C030000);

/* WALL CLIMB CODE */

// 8025e560 <- possibly resize float by char size mul for height
// 8025e5bc <- gate sound?

// 80415DCC <- this float controls how far into the wall mario is placed on
// first grab 80415DD4 <- this float controls how high mario is placed from the
// floor on first grab

// 80415DEC <- this float controls the climbing speed, scale accordingly

#if 0
static f32 scaleClimbSpeed(f32 speed) {
  TMario *player;
  SMS_FROM_GPR(30, player);


  f32 _f0;
  f32 _f3;
  f32 _f7;

  SMS_FROM_FPR(0, _f0);
  SMS_FROM_FPR(3, _f3);
  SMS_FROM_FPR(7, _f7);

  TPlayerData *playerData =
      TGlobals::getPlayerData(player);

  f32 scale = 0.015625f;

  if (playerData->isMario())
    scale *= playerData->getParams()->mSpeedMultiplier.get();

  SMS_TO_FPR(0, _f0);
  SMS_TO_FPR(3, _f3);
  SMS_TO_FPR(7, _f7);

  return scale;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8025E19C, 0, 0, 0), scaleClimbSpeed);
SMS_WRITE_32(SMS_PORT_REGION(0x8025E1A0, 0, 0, 0), 0x807E0010);
SMS_WRITE_32(SMS_PORT_REGION(0x8025E1A4, 0, 0, 0), 0x801E0014);
SMS_WRITE_32(SMS_PORT_REGION(0x8025E1C4, 0, 0, 0), 0xC0440014);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025E218, 0, 0, 0), scaleClimbSpeed);
SMS_WRITE_32(SMS_PORT_REGION(0x8025E21C, 0, 0, 0), 0x7FC3F378);

static TBGCheckData *checkClimbingWallPlane(TMario *player,
                                            TVec3f pos, f32 w,
                                            f32 h) {
  return player->checkWallPlane(pos, w * player->mScale.z, h * player->mScale.y);
}
kmCall(0x8025DD84, &checkClimbingWallPlane);
kmCall(0x8025DEB8, &checkClimbingWallPlane);
kmCall(0x8025E184, &checkClimbingWallPlane);
kmCall(0x8025E2D0, &checkClimbingWallPlane);
kmCall(0x8025E2E8, &checkClimbingWallPlane);
#endif

#if 0
static bool canJumpClingWall(TBGCheckData *wall, u16 colType) {
  TMario *player;
  SMS_FROM_GPR(28, player);

  if (colType == 266)
    return true;

  TPlayerData *playerData =
      TGlobals::getPlayerData(player);

  if (playerData->isMario() &&
      playerData->getParams()->mCanClimbWalls.get() &&
      player->mController->mButtons.mInput & TMarioGamePad::EButtons::Z)
    return true;

  return false;
}
SMS_WRITE_32(SMS_PORT_REGION(0x8024C888, 0, 0, 0), 0xA0830000);
SMS_PATCH_BL(SMS_PORT_REGION(0x8024C88C, 0, 0, 0), canJumpClingWall);
SMS_WRITE_32(SMS_PORT_REGION(0x8024C890, 0, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x8024C894, 0, 0, 0), 0x807C00D8);
SMS_WRITE_32(SMS_PORT_REGION(0x8024C898, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8024C89C, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8024C8A0, 0, 0, 0), 0x60000000);

static bool canUnkActionWall(TBGCheckData *wall, u16 colType) {
  TMario *player;
  SMS_FROM_GPR(22, player);

  if (colType == 266)
    return true;

  TPlayerData *playerData =
      TGlobals::getPlayerData(player);

  if (playerData->isMario() &&
      playerData->getParams()->mCanClimbWalls.get() &&
      player->mController->mButtons.mInput & TMarioGamePad::EButtons::Z)
    return true;

  return false;
}
SMS_WRITE_32(SMS_PORT_REGION(0x80256A3C, 0, 0, 0), 0xA0830000);
SMS_PATCH_BL(SMS_PORT_REGION(0x80256A40, 0, 0, 0), canUnkActionWall);
SMS_WRITE_32(SMS_PORT_REGION(0x80256A44, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80256A48, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80256A4C, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80256A50, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x80256A54, 0, 0, 0), 0x2C030000);

static bool canRunClingWall(TBGCheckData *wall, u16 colType) {
  TMario *player;
  SMS_FROM_GPR(31, player);

  if (colType == 266)
    return true;

  TPlayerData *playerData =
      TGlobals::getPlayerData(player);

  if (playerData->isMario() &&
      playerData->getParams()->mCanClimbWalls.get() &&
      player->mController->mButtons.mInput & TMarioGamePad::EButtons::Z)
    return true;

  return false;
}
SMS_WRITE_32(SMS_PORT_REGION(0x8025B1FC, 0, 0, 0), 0xA0830000);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025B200, 0, 0, 0), canRunClingWall);
SMS_WRITE_32(SMS_PORT_REGION(0x8025B204, 0, 0, 0), 0x807F00D8);
SMS_WRITE_32(SMS_PORT_REGION(0x8025B208, 0, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x8025B20C, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8025B210, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8025B214, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8025B218, 0, 0, 0), 0x60000000);

static bool canMoveOnWall1(u16 colType) {
  TMario *player;
  SMS_FROM_GPR(30, player);

  TBGCheckData *wall;
  SMS_FROM_GPR(29, wall);

  if (colType == 266)
    return true;

  TPlayerData *playerData =
      TGlobals::getPlayerData(player);

  if (playerData->isMario() &&
      playerData->getParams()->mCanClimbWalls.get())
    return true;

  return false;
}
SMS_WRITE_32(SMS_PORT_REGION(0x8025E2F4, 0, 0, 0), 0xA0830000);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025E2F8, 0, 0, 0), canRunClingWall);
SMS_WRITE_32(SMS_PORT_REGION(0x8025E2FC, 0, 0, 0), 0x2C030000);
SMS_WRITE_32(SMS_PORT_REGION(0x8025E300, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8025E304, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8025E308, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8025E30C, 0, 0, 0), 0x60000000);

static bool canMoveOnWall2(TBGCheckData *wall, u16 colType) {
  TMario *player;
  SMS_FROM_GPR(30, player);

  if (colType == 266)
    return true;

  TPlayerData *playerData =
      TGlobals::getPlayerData(player);

  if (playerData->isMario() &&
      playerData->getParams()->mCanClimbWalls.get())
    return true;

  return false;
}
kmCall(0x8025E31C, &canMoveOnWall2);
kmWrite32(0x8025E320, 0x2C040000);
kmWrite32(0x8025E324, 0x4182000C);

static TBGCheckData *canClimbUnderwater(TBGCheckData *wall) {
  bool canCling;
  __asm { mr canCling, r4}
  ;

  TMario *player;
  __asm { mr player, r31}
  ;

  if (playerData->isMario())
    canCling =
        wall->mType == 266 ||
        (playerData->mParams->Attributes.mCanClimbWalls &&
         player->mController->mButtons.mInput & TMarioGamePad::EButtons::Z &&
         wall->mType != 5);
  else
    canCling = wall->mType == 266;

  SMS_ASM_BLOCK {mr r4, canCling};

  return wall;
}
kmCall(0x80272660, &canClimbUnderwater);
kmWrite32(0x80272664, 0x2C040000);
kmWrite32(0x80272668, 0x4182000C);
#endif