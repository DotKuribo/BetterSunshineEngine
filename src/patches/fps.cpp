#include <Dolphin/types.h>

#include <JSystem/JDrama/JDRDirector.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>

#include <SMS/game/MarDirector.hxx>
#include <SMS/raw_fn.hxx>

#include "p_settings.hxx"

extern FPSSetting gFPSSetting;

void updateFPS(TMarDirector *director) {
    switch (gFPSSetting.getInt()) {
    case FPSSetting::FPS_30:
        gpApplication.mDisplay->mRetraceCount                                   = 2;
        *reinterpret_cast<f32 *>(SMS_PORT_REGION(0x804167B8, 0x8040DD10, 0, 0)) = 0.5f;
        *reinterpret_cast<f32 *>(SMS_PORT_REGION(0x80414904, 0x8040BE54, 0, 0)) = 0.01f;
        break;
    case FPSSetting::FPS_60:
        gpApplication.mDisplay->mRetraceCount                                   = 1;
        *reinterpret_cast<f32 *>(SMS_PORT_REGION(0x804167B8, 0x8040DD10, 0, 0)) = 1.0f;
        *reinterpret_cast<f32 *>(SMS_PORT_REGION(0x80414904, 0x8040BE54, 0, 0)) = 0.02f;
        break;
    case FPSSetting::FPS_120:
        gpApplication.mDisplay->mRetraceCount                                   = 0;
        *reinterpret_cast<f32 *>(SMS_PORT_REGION(0x804167B8, 0x8040DD10, 0, 0)) = 2.0f;
        *reinterpret_cast<f32 *>(SMS_PORT_REGION(0x80414904, 0x8040BE54, 0, 0)) = 0.04f;
        break;
    }
}

static f32 setBoidSpeed(f32 dot) {
    return sqrtf(dot) * (SMS_PORT_REGION(30.0f, 25.0f, 30.0f, 30.0f) / BetterSMS::getFrameRate());
}
SMS_PATCH_BL(SMS_PORT_REGION(0x800066E4, 0x800066E4, 0, 0), setBoidSpeed);