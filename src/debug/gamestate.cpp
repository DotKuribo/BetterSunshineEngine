#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DPane.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Player/Mario.hxx>

#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/NPC/NpcBase.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/raw_fn.hxx>

#include "debug.hxx"
#include "libs/cheathandler.hxx"
#include "libs/constmath.hxx"
#include "libs/geometry.hxx"
#include "libs/profiler.hxx"
#include "logging.hxx"
#include "module.hxx"

#include "p_debug.hxx"

using namespace BetterSMS;
using namespace BetterSMS::Geometry;

extern int gDebugUIPage;

static s16 gMonitorX = 10, gMonitorY = 180;
static s16 gFontWidth = 11, gFontHeight = 11;

static J2DTextBox *gpPlayerStateStringW    = nullptr;
static J2DTextBox *gpPlayerStateStringB    = nullptr;
static J2DTextBox *gpWorldStateStringW     = nullptr;
static J2DTextBox *gpWorldStateStringB     = nullptr;
static J2DTextBox *gpCollisionStateStringW = nullptr;
static J2DTextBox *gpCollisionStateStringB = nullptr;
static J2DTextBox *gpCameraStateStringW    = nullptr;
static J2DTextBox *gpCameraStateStringB    = nullptr;

static char sPlayerStringBuffer[300]{};
static char sWorldStringBuffer[300]{};
static char sCollisionStringBuffer[350]{};
static char sCameraStringBuffer[200]{};

static size_t sHitObjCount = 0;
static bool sIsInitialized = false;

static void addToHitActorCount(JDrama::TViewObj *obj, u32 flags, JDrama::TGraphics *graphics) {
    testPerform__Q26JDrama8TViewObjFUlPQ26JDrama9TGraphics(obj, flags, graphics);
    ++sHitObjCount;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802A0450, 0, 0, 0), addToHitActorCount);

BETTER_SMS_FOR_CALLBACK void initGameStateMonitor(TApplication *app) {
    gDebugUIPage = 1;

    gpPlayerStateStringW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpPlayerStateStringB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpPlayerStateStringW->mStrPtr         = sPlayerStringBuffer;
    gpPlayerStateStringB->mStrPtr         = sPlayerStringBuffer;
    gpPlayerStateStringW->mNewlineSize    = gFontHeight;
    gpPlayerStateStringW->mCharSizeX      = gFontWidth;
    gpPlayerStateStringW->mCharSizeY      = gFontHeight;
    gpPlayerStateStringB->mNewlineSize    = gFontHeight;
    gpPlayerStateStringB->mCharSizeX      = gFontWidth;
    gpPlayerStateStringB->mCharSizeY      = gFontHeight;
    gpPlayerStateStringW->mGradientTop    = {255, 255, 255, 255};
    gpPlayerStateStringW->mGradientBottom = {255, 255, 255, 255};
    gpPlayerStateStringB->mGradientTop    = {0, 0, 0, 255};
    gpPlayerStateStringB->mGradientBottom = {0, 0, 0, 255};

    gpWorldStateStringW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpWorldStateStringB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpWorldStateStringW->mStrPtr         = sWorldStringBuffer;
    gpWorldStateStringB->mStrPtr         = sWorldStringBuffer;
    gpWorldStateStringW->mNewlineSize    = gFontHeight;
    gpWorldStateStringW->mCharSizeX      = gFontWidth;
    gpWorldStateStringW->mCharSizeY      = gFontHeight;
    gpWorldStateStringB->mNewlineSize    = gFontHeight;
    gpWorldStateStringB->mCharSizeX      = gFontWidth;
    gpWorldStateStringB->mCharSizeY      = gFontHeight;
    gpWorldStateStringW->mGradientTop    = {255, 255, 255, 255};
    gpWorldStateStringW->mGradientBottom = {255, 255, 255, 255};
    gpWorldStateStringB->mGradientTop    = {0, 0, 0, 255};
    gpWorldStateStringB->mGradientBottom = {0, 0, 0, 255};

    gpCollisionStateStringW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpCollisionStateStringB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpCollisionStateStringW->mStrPtr         = sCollisionStringBuffer;
    gpCollisionStateStringB->mStrPtr         = sCollisionStringBuffer;
    gpCollisionStateStringW->mNewlineSize    = gFontHeight;
    gpCollisionStateStringW->mCharSizeX      = gFontWidth;
    gpCollisionStateStringW->mCharSizeY      = gFontHeight;
    gpCollisionStateStringB->mNewlineSize    = gFontHeight;
    gpCollisionStateStringB->mCharSizeX      = gFontWidth;
    gpCollisionStateStringB->mCharSizeY      = gFontHeight;
    gpCollisionStateStringW->mGradientTop    = {255, 255, 255, 255};
    gpCollisionStateStringW->mGradientBottom = {255, 255, 255, 255};
    gpCollisionStateStringB->mGradientTop    = {0, 0, 0, 255};
    gpCollisionStateStringB->mGradientBottom = {0, 0, 0, 255};

    gpCameraStateStringW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpCameraStateStringB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpCameraStateStringW->mStrPtr         = sCameraStringBuffer;
    gpCameraStateStringB->mStrPtr         = sCameraStringBuffer;
    gpCameraStateStringW->mNewlineSize    = gFontHeight;
    gpCameraStateStringW->mCharSizeX      = gFontWidth;
    gpCameraStateStringW->mCharSizeY      = gFontHeight;
    gpCameraStateStringB->mNewlineSize    = gFontHeight;
    gpCameraStateStringB->mCharSizeX      = gFontWidth;
    gpCameraStateStringB->mCharSizeY      = gFontHeight;
    gpCameraStateStringW->mGradientTop    = {255, 255, 255, 255};
    gpCameraStateStringW->mGradientBottom = {255, 255, 255, 255};
    gpCameraStateStringB->mGradientTop    = {0, 0, 0, 255};
    gpCameraStateStringB->mGradientBottom = {0, 0, 0, 255};

    sIsInitialized = true;
}

BETTER_SMS_FOR_CALLBACK void updateGameStateMonitor(TApplication *app) {
    TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

    if (app->mContext != TApplication::CONTEXT_DIRECT_STAGE || !gpMarioAddress || !sIsInitialized)
        return;

    if (director->mCurState == TMarDirector::STATE_INTRO_INIT)
        return;

    u16 floorColType      = gpMarioAddress->mFloorTriangle ? gpMarioAddress->mFloorTriangle->mType
                                                           : 0xFFFF;
    u16 floorColValue     = gpMarioAddress->mFloorTriangle ? gpMarioAddress->mFloorTriangle->mValue
                                                           : 0xFFFF;
    TVec3f floorColNormal = gpMarioAddress->mFloorTriangle ? gpMarioAddress->mFloorTriangle->mNormal
                                                           : TVec3f(0.0f, 0.0f, 0.0f);
    u16 wallColType = gpMarioAddress->mWallTriangle ? gpMarioAddress->mWallTriangle->mType : 0xFFFF;
    u16 wallColValue     = gpMarioAddress->mWallTriangle ? gpMarioAddress->mWallTriangle->mValue
                                                         : 0xFFFF;
    TVec3f wallColNormal = gpMarioAddress->mWallTriangle ? gpMarioAddress->mWallTriangle->mNormal
                                                         : TVec3f(0.0f, 0.0f, 0.0f);
    u16 roofColType = gpMarioAddress->mRoofTriangle ? gpMarioAddress->mRoofTriangle->mType : 0xFFFF;
    u16 roofColValue     = gpMarioAddress->mRoofTriangle ? gpMarioAddress->mRoofTriangle->mValue
                                                         : 0xFFFF;
    TVec3f roofColNormal = gpMarioAddress->mRoofTriangle ? gpMarioAddress->mRoofTriangle->mNormal
                                                         : TVec3f(0.0f, 0.0f, 0.0f);

    snprintf(sPlayerStringBuffer, 300,
             "Player Stats:\n"
             "  Position:      %.02f, %.02f, %.02f\n"
             "  Rotation:      %.02f, %.02f, %.02f\n"
             "  Movement:    %.02f, %.02f, %.02f\n"
             "  Speed:        %.02f\n"
             "  Status:        0x%lX\n"
             "  State:         0x%lX\n"
             "  Flags:         0x%lX\n"
             "  Animation:     %d\n"
             "  Animation FPS: %.02f\n",
             gpMarioAddress->mTranslation.x, gpMarioAddress->mTranslation.y,
             gpMarioAddress->mTranslation.z, gpMarioAddress->mRotation.x,
             gpMarioAddress->mRotation.y, gpMarioAddress->mRotation.z, gpMarioAddress->mSpeed.x,
             gpMarioAddress->mSpeed.y, gpMarioAddress->mSpeed.z, gpMarioAddress->mForwardSpeed,
             gpMarioAddress->mState, gpMarioAddress->mActionState,
             *reinterpret_cast<u32 *>(&gpMarioAddress->mAttributes), gpMarioAddress->mAnimationID,
             gpMarioAddress->mModelData->mFrameCtrl->mFrameRate);

    snprintf(sWorldStringBuffer, 300,
             "World Stats:\n"
             "  Area ID:        %d\n"
             "  Episode ID:     %d\n"
             "  Warp ID:        0x%X\n"
             "  Perform Objs:  %lu\n",
             director->mAreaID, director->mEpisodeID,
             ((director->mAreaID + 1) << 8) | director->mEpisodeID, sHitObjCount);

    snprintf(sCollisionStringBuffer, 350,
             "Collision Stats:\n"
             "  Triangles:       %lu\n"
             "  Static Lists:    %lu\n"
             "  Move Lists:     %lu\n"
             "  Warp Lists:     %d\n"
             "  Floor Normal:   %.02f, %.02f, %.02f\n"
             "  Floor Type:     0x%hX\n"
             "  Floor Value:    0x%hX\n"
             "  Wall Normal:    %.02f, %.02f, %.02f\n"
             "  Wall Type:      0x%hX\n"
             "  Wall Value:     0x%hX\n",
             gpMapCollisionData->mCheckDataCount, gpMapCollisionData->mCheckListStaticCount,
             gpMapCollisionData->mCheckListMax - gpMapCollisionData->mCheckListMoveRemaining,
             gpMapCollisionData->mCheckListWarpCount, floorColNormal.x, floorColNormal.y,
             floorColNormal.z, floorColType, floorColValue, wallColNormal.x, wallColNormal.y,
             wallColNormal.z, wallColType, wallColValue);

    TVec3f camRotation = Vector3::eulerFromMatrix(gpCamera->mTRSMatrix);

    snprintf(sCameraStringBuffer, 200,
             "Camera Stats:\n"
             "  Position:   %.02f, %.02f, %.02f\n"
             "  Rotation:   %.02f, %.02f, %.02f\n"
             "  Aspect:     %.02f\n"
             "  FOV:        %.02f\n",
             gpCamera->mTranslation.x, gpCamera->mTranslation.y, gpCamera->mTranslation.z,
             camRotation.x, camRotation.y, camRotation.z, gpCamera->mProjectionAspect,
             gpCamera->mProjectionFovy);

    sHitObjCount = 0;
}

BETTER_SMS_FOR_CALLBACK void drawGameStateMonitor(TApplication *app, const J2DOrthoGraph *ortho) {
    TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

    if (app->mContext != TApplication::CONTEXT_DIRECT_STAGE || !gpMarioAddress || !sIsInitialized)
        return;

    if (director->mCurState == TMarDirector::STATE_INTRO_INIT)
        return;

    {
        s16 adjust = getScreenRatioAdjustX();
        switch (gDebugUIPage) {
        case 1:
            gpPlayerStateStringB->draw(gMonitorX - adjust + 1, gMonitorY + 1);
            gpPlayerStateStringW->draw(gMonitorX - adjust, gMonitorY);
            break;
        case 2:
            gpWorldStateStringB->draw(gMonitorX - adjust + 1, gMonitorY + 1);
            gpWorldStateStringW->draw(gMonitorX - adjust, gMonitorY);
            break;
        case 3:
            gpCollisionStateStringB->draw(gMonitorX - adjust + 1, gMonitorY + 1);
            gpCollisionStateStringW->draw(gMonitorX - adjust, gMonitorY);
            break;
        case 4:
            gpCameraStateStringB->draw(gMonitorX - adjust + 1, gMonitorY + 1);
            gpCameraStateStringW->draw(gMonitorX - adjust, gMonitorY);
            break;
        default:
            break;
        }
    }
}