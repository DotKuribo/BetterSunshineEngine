#include <Dolphin/types.h>

#include <SMS/camera/CameraMarioData.hxx>
#include <SMS/camera/PolarSubCamera.hxx>
#include <SMS/camera/LensFlare.hxx>
#include <SMS/camera/LensGlow.hxx>
#include <SMS/camera/SunModel.hxx>
#include <SMS/manager/ModelWaterManager.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "common_sdk.h"
#include "libs/constmath.hxx"
#include "module.hxx"

#if BETTER_SMS_BUGFIXES

using namespace BetterSMS;

static TVec3f sSunBasePos(0.0f, 0.0f, 0.0f);
static u8 sSunBaseBlindStrength = 0;

static s16 captureSunData() {
    TSunModel *sun;
    SMS_FROM_GPR(30, sun);

    TVec3f spos;

    JSGGetTranslation__Q26JDrama6TActorCFP3Vec(sun, reinterpret_cast<Vec *>(&spos));
    sSunBasePos.set(spos);
    sSunBaseBlindStrength = sun->mBlindingStrength;

    return SMSGetAnmFrameRate__Fv();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8002F440, 0x8002F4F8, 0, 0), captureSunData);

static bool sunFollowCameraAndScaleLightness(TCameraMarioData *cam) {
    TSunModel *sun;
    SMS_FROM_GPR(29, sun);

    TVec3f spos(sSunBasePos);
    TVec3f cpos;

    JSGGetViewPosition__Q26JDrama13TLookAtCameraCFP3Vec(gpCamera, reinterpret_cast<Vec *>(&cpos));

    spos.add(cpos);
    JSGSetTranslation__Q26JDrama6TActorFRC3Vec(sun, reinterpret_cast<Vec &>(spos));

    sun->mBlindingStrength = lerp<u8>(20, sSunBaseBlindStrength,
                                      static_cast<f32>(gpModelWaterManager->mDarkLevel) / 255.0f);

    return cam->isMarioIndoor();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8002EB34, 0x8002EBEC, 0, 0), sunFollowCameraAndScaleLightness);

static f32 scaleFlareToLightness(f32 a, f32 b, f32 c) {
    return CLBEaseOutInbetween_f(a, static_cast<f32>(gpModelWaterManager->mDarkLevel), c);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8002D358, 0x8002D410, 0, 0), scaleFlareToLightness);

static bool scaleGlowToLightness(f32 a, f32 b, f32 c) {
    TLensGlow *glow;
    SMS_FROM_GPR(28, glow);

    const f32 factor = static_cast<f32>(gpModelWaterManager->mDarkLevel) / 255.0f;

    return CLBLinearInbetween_f(a * factor, b * factor, c);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8002DB60, 0x8002DC18, 0, 0), scaleGlowToLightness);

#endif
