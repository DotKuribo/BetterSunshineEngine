#include <Dolphin/types.h>

#include <SMS/actor/HitActor.hxx>
#include <SMS/actor/LiveActor.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "libs/constmath.hxx"
#include "common_sdk.h"
#include "module.hxx"
#include "p_settings.hxx"
#include "player.hxx"

using namespace BetterSMS;

#if BETTER_SMS_EXTENDED_RENDER_DISTANCE

constexpr f32 DrawDistanceMultiplier = 100.0f;
constexpr f32 DrawDistance           = 300000.0f * DrawDistanceMultiplier;

static bool cameraScaleRenderDistance(CPolarSubCamera *cam) {
    JSGSetProjectionFar__Q26JDrama7TCameraFf(cam, BetterSMS::areBugsPatched() ? DrawDistance : 300000.0f);  // Draw Distance
    return cam->isNormalDeadDemo();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80023828, 0x800238e0, 0, 0), cameraScaleRenderDistance);

static void scaleDrawDistanceNPC(f32 x, f32 y, f32 near, f32 far) {
    SetViewFrustumClipCheckPerspective__Fffff(x, y, near, BetterSMS::areBugsPatched() ? far * DrawDistanceMultiplier : far);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8020A2A4, 0x80202188, 0, 0), scaleDrawDistanceNPC);

#endif

using namespace BetterSMS;

static f32 sLastFactor = 1.0f;
static void updateSlideFOV(CPolarSubCamera *cam) {
    f32 factor = scaleLinearAtAnchor<f32>(gpMarioAddress->mForwardSpeed / 100.0f, 0.5f, 1.0f);

    factor = lerp<f32>(sLastFactor, factor, 0.01f);

    if (factor > 1.0f && gpMarioAddress->mState == static_cast<u32>(TMario::STATE_DIVESLIDE)) {
        sLastFactor = factor;
        reinterpret_cast<f32 *>(cam)[0x48 / 4] *= factor;
    } else {
        sLastFactor = lerp<f32>(sLastFactor, 1.0f, 0.01f);
        reinterpret_cast<f32 *>(cam)[0x48 / 4] *= sLastFactor;
    }
}

static bool updateContexts(CPolarSubCamera *cam) {
    updateSlideFOV(cam);
    return cam->isNormalDeadDemo();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80023598, 0x80023650, 0, 0), updateContexts);

/* HOOKS */

static void modifyCameraRangeToSize(f32 *cameraParams, f32 *saveParams) {
    cameraParams[0xA8 / 4] = saveParams[0x3B0 / 4];

    // Custom code here
    auto data = Player::getRegisteredData(gpMarioAddress, "__better_sms");
    if (!data)
        return;

    auto *playerData = reinterpret_cast<Player::TPlayerData *>(data);
    if (!playerData)
        return;

    auto *params = playerData->getParams();
    if (!params)
        return;

    const f32 scale = playerData->getParams()->mSizeMultiplier.get();

    if (!gpMarioAddress->mYoshi || gpMarioAddress->mYoshi->mState != TYoshi::MOUNTED ||
        scale > 1.0f) {
        cameraParams[0x8 / 4] *= scaleLinearAtAnchor<f32>(scale, 0.5f, 1.0f);
        cameraParams[0xC / 4] *= scaleLinearAtAnchor<f32>(scale, 0.5f, 1.0f);
        cameraParams[0x24 / 4] *= scaleLinearAtAnchor<f32>(scale, 0.9375f, 1.0f);
    }
}
SMS_PATCH_B(SMS_PORT_REGION(0x80027548, 0x80027600, 0, 0), modifyCameraRangeToSize);

static void checkInvertCamCStickX(CPolarSubCamera *camera, f32 x) {
    camera->rotateY_ByStickX_(BetterSMS::isCameraInvertedX() ? -x : x);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80029EAC, 0, 0, 0), checkInvertCamCStickX);

static void checkInvertCamCStickY(CPolarSubCamera *camera, f32 y) {
    camera->rotateX_ByStickY_(BetterSMS::isCameraInvertedY() ? -y : y);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8002A1E0, 0, 0, 0), checkInvertCamCStickY);