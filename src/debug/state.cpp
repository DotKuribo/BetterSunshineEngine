#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DPane.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/SMS.hxx>
#include <SMS/actor/Mario.hxx>
#include <SMS/npc/BaseNPC.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/sound/MSound.hxx>
#include <SMS/sound/MSoundSESystem.hxx>

#include "debug.hxx"
#include "libs/cheathandler.hxx"
#include "libs/constmath.hxx"
#include "logging.hxx"
#include "module.hxx"

using namespace BetterSMS;

#if BETTER_SMS_WIDESCREEN
static s16 gPlayerMonitorX = -86, gPlayerMonitorY = 190;
static s16 gWorldMonitorX = -86, gWorldMonitorY = 310;
static s16 gCollisionMonitorX = 430, gCollisionMonitorY = 190;
#else
static s16 gPlayerMonitorX = 10, gPlayerMonitorY = 190;
static s16 gWorldMonitorX = 10, gWorldMonitorY = 310;
static s16 gCollisionMonitorX = 490, gCollisionMonitorY = 190;
#endif

static J2DTextBox *gpPlayerStateStringW    = nullptr;
static J2DTextBox *gpPlayerStateStringB    = nullptr;
static J2DTextBox *gpWorldStateStringW     = nullptr;
static J2DTextBox *gpWorldStateStringB     = nullptr;
static J2DTextBox *gpCollisionStateStringW = nullptr;
static J2DTextBox *gpCollisionStateStringB = nullptr;

static char sPlayerStringBuffer[300]{};
static char sWorldStringBuffer[300]{};
static char sCollisionStringBuffer[300]{};

static size_t sHitObjCount = 0;

void addToHitActorCount(JDrama::TViewObj *obj, u32 flags, JDrama::TGraphics *graphics) {
    testPerform__Q26JDrama8TViewObjFUlPQ26JDrama9TGraphics(obj, flags, graphics);
    ++sHitObjCount;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802a0450, 0, 0, 0), addToHitActorCount);

void initStateMonitor(TMarDirector *director) {
    gpPlayerStateStringW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpPlayerStateStringB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpPlayerStateStringW->mStrPtr         = sPlayerStringBuffer;
    gpPlayerStateStringB->mStrPtr         = sPlayerStringBuffer;
    gpPlayerStateStringW->mNewlineSize    = 14;
    gpPlayerStateStringW->mCharSizeX      = 12;
    gpPlayerStateStringW->mCharSizeY      = 14;
    gpPlayerStateStringB->mNewlineSize    = 14;
    gpPlayerStateStringB->mCharSizeX      = 12;
    gpPlayerStateStringB->mCharSizeY      = 14;
    gpPlayerStateStringW->mGradientTop    = {255, 255, 255, 255};
    gpPlayerStateStringW->mGradientBottom = {255, 255, 255, 255};
    gpPlayerStateStringB->mGradientTop    = {0, 0, 0, 255};
    gpPlayerStateStringB->mGradientBottom = {0, 0, 0, 255};

    gpWorldStateStringW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpWorldStateStringB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpWorldStateStringW->mStrPtr         = sWorldStringBuffer;
    gpWorldStateStringB->mStrPtr         = sWorldStringBuffer;
    gpWorldStateStringW->mNewlineSize    = 14;
    gpWorldStateStringW->mCharSizeX      = 12;
    gpWorldStateStringW->mCharSizeY      = 14;
    gpWorldStateStringB->mNewlineSize    = 14;
    gpWorldStateStringB->mCharSizeX      = 12;
    gpWorldStateStringB->mCharSizeY      = 14;
    gpWorldStateStringW->mGradientTop    = {255, 255, 255, 255};
    gpWorldStateStringW->mGradientBottom = {255, 255, 255, 255};
    gpWorldStateStringB->mGradientTop    = {0, 0, 0, 255};
    gpWorldStateStringB->mGradientBottom = {0, 0, 0, 255};

    gpCollisionStateStringW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpCollisionStateStringB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpCollisionStateStringW->mStrPtr         = sCollisionStringBuffer;
    gpCollisionStateStringB->mStrPtr         = sCollisionStringBuffer;
    gpCollisionStateStringW->mNewlineSize    = 14;
    gpCollisionStateStringW->mCharSizeX      = 12;
    gpCollisionStateStringW->mCharSizeY      = 14;
    gpCollisionStateStringB->mNewlineSize    = 14;
    gpCollisionStateStringB->mCharSizeX      = 12;
    gpCollisionStateStringB->mCharSizeY      = 14;
    gpCollisionStateStringW->mGradientTop    = {255, 255, 255, 255};
    gpCollisionStateStringW->mGradientBottom = {255, 255, 255, 255};
    gpCollisionStateStringB->mGradientTop    = {0, 0, 0, 255};
    gpCollisionStateStringB->mGradientBottom = {0, 0, 0, 255};
}

void updateStateMonitor(TMarDirector *director) {
    if (!director || !gpMarioAddress || !gpPlayerStateStringW || !gpPlayerStateStringB)
        return;

    if (director->mCurState != TMarDirector::Status::NORMAL &&
        director->mCurState != TMarDirector::Status::PAUSE_MENU)
        return;

    u16 floorColType =
        gpMarioAddress->mFloorTriangle ? gpMarioAddress->mFloorTriangle->mCollisionType : 0xFFFF;
    u16 floorColValue     = gpMarioAddress->mFloorTriangle ? gpMarioAddress->mFloorTriangle->mValue4
                                                           : 0xFFFF;
    TVec3f floorColNormal = gpMarioAddress->mFloorTriangle ? gpMarioAddress->mFloorTriangle->mNormal
                                                           : TVec3f(0.0f, 0.0f, 0.0f);
    u16 wallColType  = gpMarioAddress->mWallTriangle ? gpMarioAddress->mWallTriangle->mCollisionType
                                                     : 0xFFFF;
    u16 wallColValue = gpMarioAddress->mWallTriangle ? gpMarioAddress->mWallTriangle->mValue4
                                                     : 0xFFFF;
    TVec3f wallColNormal = gpMarioAddress->mWallTriangle ? gpMarioAddress->mWallTriangle->mNormal
                                                         : TVec3f(0.0f, 0.0f, 0.0f);
    u16 roofColType  = gpMarioAddress->mRoofTriangle ? gpMarioAddress->mRoofTriangle->mCollisionType
                                                     : 0xFFFF;
    u16 roofColValue = gpMarioAddress->mRoofTriangle ? gpMarioAddress->mRoofTriangle->mValue4
                                                     : 0xFFFF;
    TVec3f roofColNormal = gpMarioAddress->mRoofTriangle ? gpMarioAddress->mRoofTriangle->mNormal
                                                         : TVec3f(0.0f, 0.0f, 0.0f);

    snprintf(sPlayerStringBuffer, 300,
             "Player Stats:\n"
             "  Position:   %.02f, %.02f, %.02f\n"
             "  Rotation:   %.02f, %.02f, %.02f\n"
             "  Movement: %.02f, %.02f, %.02f\n"
             "  Speed:     %.02f\n"
             "  Status:     0x%lX\n"
             "  State:      0x%lX\n"
             "  Flags:      0x%lX",
             gpMarioAddress->mPosition.x, gpMarioAddress->mPosition.y, gpMarioAddress->mPosition.z,
             gpMarioAddress->mRotation.x, gpMarioAddress->mRotation.y, gpMarioAddress->mRotation.z,
             gpMarioAddress->mSpeed.x, gpMarioAddress->mSpeed.y, gpMarioAddress->mSpeed.z,
             gpMarioAddress->mForwardSpeed, gpMarioAddress->mState, gpMarioAddress->mActionState,
             *reinterpret_cast<u32 *>(&gpMarioAddress->mAttributes));

    snprintf(sWorldStringBuffer, 300,
             "World Stats:\n"
             "  Area ID:        %d\n"
             "  Episode ID:     %d\n"
             "  Warp ID:        0x%X\n"
             "  Perform Objs:  %lu\n",
             director->mAreaID, director->mEpisodeID,
             ((director->mAreaID + 1) << 8) | director->mEpisodeID, sHitObjCount);

    snprintf(sCollisionStringBuffer, 300,
             "Collision Stats:\n"
             "  Collision Tris:   %lu\n"
             "  Floor Normal:   %.02f, %.02f, %.02f\n"
             "  Floor Type:     0x%hX\n"
             "  Floor Value:    0x%hX\n"
             "  Wall Normal:    %.02f, %.02f, %.02f\n"
             "  Wall Type:      0x%hX\n"
             "  Wall Value:     0x%hX\n"
             "  Roof Normal:   %.02f, %.02f, %.02f\n"
             "  Roof Type:     0x%hX\n"
             "  Roof Value:    0x%hX\n",
             gpMapCollisionData->mCheckDataLength, floorColNormal.x, floorColNormal.y,
             floorColNormal.z, floorColType, floorColValue, wallColNormal.x, wallColNormal.y,
             wallColNormal.z, wallColType, wallColValue, roofColNormal.x, roofColNormal.y,
             roofColNormal.z, roofColType, roofColValue);

    sHitObjCount = 0;
}

void drawStateMonitor(TMarDirector *director, J2DOrthoGraph *ortho) {
    gpPlayerStateStringB->draw(gPlayerMonitorX + 1, gPlayerMonitorY + 1);
    gpPlayerStateStringW->draw(gPlayerMonitorX, gPlayerMonitorY);
    gpWorldStateStringB->draw(gWorldMonitorX + 1, gWorldMonitorY + 1);
    gpWorldStateStringW->draw(gWorldMonitorX, gWorldMonitorY);
    gpCollisionStateStringB->draw(gCollisionMonitorX + 1, gCollisionMonitorY + 1);
    gpCollisionStateStringW->draw(gCollisionMonitorX, gCollisionMonitorY);
}