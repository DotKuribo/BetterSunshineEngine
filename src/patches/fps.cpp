#include <Dolphin/types.h>

#include <JSystem/JDrama/JDRDirector.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>

#include <SMS/System/MarDirector.hxx>
#include <SMS/raw_fn.hxx>

#include "p_settings.hxx"

extern FPSSetting gFPSSetting;

BETTER_SMS_FOR_CALLBACK void updateFPS(TMarDirector *director) {
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

static f32 getAnimalBirdSpeed() {
    return 2.0f;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8000CEB0, 0, 0, 0), getAnimalBirdSpeed);
SMS_PATCH_BL(SMS_PORT_REGION(0x8000D1D8, 0, 0, 0), getAnimalBirdSpeed);
SMS_PATCH_BL(SMS_PORT_REGION(0x8000D1F8, 0, 0, 0), getAnimalBirdSpeed);

static void startCameraBckWithDelta(MActor* actor, const char* bck) {
    actor->setBck(bck);
    actor->setFrameRate(SMSGetAnmFrameRate(), MActor::BCK);
}
//SMS_PATCH_BL(SMS_PORT_REGION(0x80031F60, 0, 0, 0), startCameraBckWithDelta);

static f32 getBossEelAnmFrameRate() { return 2.0f; }
SMS_PATCH_BL(SMS_PORT_REGION(0x800D059C, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D07A0, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D0898, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D0B60, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D0E0C, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D1128, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D12F0, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D147C, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D15C0, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D1C98, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D1D68, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D207C, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D2364, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D2438, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D24F8, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D2710, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D2AD8, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D2F8C, 0, 0, 0), getBossEelAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x800D3350, 0, 0, 0), getBossEelAnmFrameRate);
