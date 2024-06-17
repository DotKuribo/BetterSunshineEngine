#include <Dolphin/types.h>

#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/Strategic/HitActor.hxx>
#include <SMS/Strategic/LiveActor.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "libs/constmath.hxx"
#include "module.hxx"
#include "p_settings.hxx"
#include "player.hxx"

using namespace BetterSMS;

#if BETTER_SMS_EXTENDED_RENDER_DISTANCE

constexpr f32 DrawDistanceMultiplier = 100.0f;
constexpr f32 DrawDistance           = 300000.0f * DrawDistanceMultiplier;

static bool cameraScaleRenderDistance(CPolarSubCamera *cam) {
    JSGSetProjectionFar__Q26JDrama7TCameraFf(
        cam, BetterSMS::areBugsPatched() ? DrawDistance : 300000.0f);  // Draw Distance
    return cam->isNormalDeadDemo();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80023828, 0x800238e0, 0, 0), cameraScaleRenderDistance);

static void scaleDrawDistanceNPC(f32 x, f32 y, f32 near, f32 far) {
    SetViewFrustumClipCheckPerspective__Fffff(
        x, y, near, BetterSMS::areBugsPatched() ? far * DrawDistanceMultiplier : far);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8020A2A4, 0x80202188, 0, 0), scaleDrawDistanceNPC);

#endif

using namespace BetterSMS;

/* HOOKS */

static void checkInvertCamCStickX(CPolarSubCamera *camera, f32 x) {
    camera->rotateY_ByStickX_(BetterSMS::isCameraInvertedX() ? -x : x);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80029E44, 0, 0, 0), checkInvertCamCStickX);
SMS_PATCH_BL(SMS_PORT_REGION(0x80029EAC, 0, 0, 0), checkInvertCamCStickX);

static void checkInvertCamCStickY(CPolarSubCamera *camera, f32 y) {
    camera->rotateX_ByStickY_(BetterSMS::isCameraInvertedY() ? -y : y);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x800293B4, 0, 0, 0), checkInvertCamCStickY);
SMS_PATCH_BL(SMS_PORT_REGION(0x8002A1E0, 0, 0, 0), checkInvertCamCStickY);
SMS_PATCH_BL(SMS_PORT_REGION(0x80031104, 0, 0, 0), checkInvertCamCStickY);