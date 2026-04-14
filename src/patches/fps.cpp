#include <Dolphin/types.h>

#include <JSystem/JDrama/JDRDirector.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>

#include <SMS/GC2D/Talk2D2.hxx>
#include <SMS/System/MarDirector.hxx>
#include <SMS/raw_fn.hxx>

#include "p_settings.hxx"

extern FPSSetting gFPSSetting;

static f32 sFaderFrameRate       = 1.0f;
static f32 sShineSelectFrameRate = 1.0f;

namespace BetterSMS {

    namespace FPS {

        BETTER_SMS_FOR_EXPORT void setSMSFaderFrameRate(f32 framerate) {
            sFaderFrameRate = framerate;
        }

        BETTER_SMS_FOR_EXPORT void setShineSelectFrameRate(f32 framerate) {
            sShineSelectFrameRate = framerate;
        }

    }  // namespace FPS

}  // namespace BetterSMS

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

static f32 getAnimalBirdSpeed() { return 2.0f; }
SMS_PATCH_BL(SMS_PORT_REGION(0x8000CEB0, 0, 0, 0), getAnimalBirdSpeed);
SMS_PATCH_BL(SMS_PORT_REGION(0x8000D1D8, 0, 0, 0), getAnimalBirdSpeed);
SMS_PATCH_BL(SMS_PORT_REGION(0x8000D1F8, 0, 0, 0), getAnimalBirdSpeed);

static void startCameraBckWithDelta(MActor *actor, const char *bck) {
    actor->setBck(bck);
    actor->setFrameRate(SMSGetAnmFrameRate(), MActor::BCK);
}
// SMS_PATCH_BL(SMS_PORT_REGION(0x80031F60, 0, 0, 0), startCameraBckWithDelta);

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

// Make TJointCoin (and derived e.g. SandBird) animations consistent (delta messes it up somehow)
static void getJointCoinAnmFrameRate(J3DFrameCtrl *frameCtrl) {
    frameCtrl->mFrameRate = 2.0f * 0.25f;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801F76C0, 0, 0, 0), getJointCoinAnmFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x801F76DC, 0, 0, 0), getJointCoinAnmFrameRate);

// Fix 60fps making entering textboxes 2x speed (by doubling the timer)
static void adjustTextBoxTimer() {
    Talk2D2 *talk2D2;
    SMS_FROM_GPR(28, talk2D2);

    switch (gFPSSetting.getInt()) {
    case FPSSetting::FPS_30:
        ((u8 *)talk2D2)[0x251] = 20;
        break;
    case FPSSetting::FPS_60:
        ((u8 *)talk2D2)[0x251] = 40;
        break;
    case FPSSetting::FPS_120:
        ((u8 *)talk2D2)[0x251] = 80;
        break;
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80151D34, 0, 0, 0), adjustTextBoxTimer);

// Fix 60fps making HX wipes 2x speed (by doubling the discrete timer and halving the animation fps)
// ------------------------------------------------------------------------
static inline void HX_SetTimer(int frames) {
    *(int *)SMS_PORT_REGION(0x803F43FC, 0, 0, 0) = frames;
}
static inline void HX_SetFrameRate(f32 frameRate) {
    *(f32 *)SMS_PORT_REGION(0x8040DE90, 0, 0, 0) = frameRate;
}

static inline f32 HX_GetFrameRate() { return *(f32 *)SMS_PORT_REGION(0x8040DE90, 0, 0, 0); }

static float getFaderFrameRateMultiplierFloat() {
    if (sFaderFrameRate == 0.0f) {
        return 1.0f;
    }

    switch (gFPSSetting.getInt()) {
    case FPSSetting::FPS_30:
        return 1.0f / sFaderFrameRate;
    case FPSSetting::FPS_60:
        return 2.0f / sFaderFrameRate;
    case FPSSetting::FPS_120:
        return 4.0f / sFaderFrameRate;
    }
    return 1.0f / sFaderFrameRate;
}

// clang-format off
#define HX_PATCH(address, name, method, value)                                                     \
    static u32 name(u32 ret) { method(value); return ret; }                                                          \
    SMS_PATCH_BL(SMS_PORT_REGION(address, 0, 0, 0), name);

#define HX_PATCH_FRAME_RATE(address, name, framerate)                                              \
    HX_PATCH(address, SMS_CONCATENATE(_tm_, name), HX_SetFrameRate,                                \
             framerate / getFaderFrameRateMultiplierFloat())

#define HX_PATCH_TIMER(address, name, frames)                                                      \
    HX_PATCH(address, SMS_CONCATENATE(_tm_, name), HX_SetTimer, frames * getFaderFrameRateMultiplierFloat())
// clang-format on

// -- CIRCLE -- //
HX_PATCH_TIMER(SMS_PORT_REGION(0x80181B54, 0, 0, 0), adjustHXTimerCircle_State0, 0x19);
HX_PATCH_TIMER(SMS_PORT_REGION(0x80181B78, 0, 0, 0), adjustHXTimerCircle_State1, 0x1E);
// ----------- //

// -- TEST1 -- //
HX_PATCH_TIMER(SMS_PORT_REGION(0x8017F5BC, 0, 0, 0), adjustHXTimerTest1_State0, 0x19);
// ----------- //

// -- TEST2 -- //
HX_PATCH_TIMER(SMS_PORT_REGION(0x8017EFA0, 0, 0, 0), adjustHXTimerTest2_State0, 0xB);
HX_PATCH_TIMER(SMS_PORT_REGION(0x8017F014, 0, 0, 0), adjustHXTimerTest2_State1, 0xB);
HX_PATCH_TIMER(SMS_PORT_REGION(0x8017F0D0, 0, 0, 0), adjustHXTimerTest2_State2, 0xA);
HX_PATCH_TIMER(SMS_PORT_REGION(0x8017F1C0, 0, 0, 0), adjustHXTimerTest2_State3, 0xC);
// ----------- //

// -- TEST2R -- //
HX_PATCH_TIMER(SMS_PORT_REGION(0x8017EB74, 0, 0, 0), adjustHXTimerTest2R_State0, 0xB);
HX_PATCH_TIMER(SMS_PORT_REGION(0x8017EC90, 0, 0, 0), adjustHXTimerTest2R_State1, 0xB);
HX_PATCH_TIMER(SMS_PORT_REGION(0x8017ED90, 0, 0, 0), adjustHXTimerTest2R_State2, 0xA);
HX_PATCH_TIMER(SMS_PORT_REGION(0x8017EE5C, 0, 0, 0), adjustHXTimerTest2R_State3, 0xC);
// ------------ //

// -- TEST4 -- //
static f32 sHXTimerAsFloat = 0.0f;  // We use this to make the steps consistent across frame rates,
                                    // since the timer is stored as int

static void adjustHXFrameRateTest4_State0() {
    HX_SetFrameRate(5.0f / getFaderFrameRateMultiplierFloat());
    *(f32 *)SMS_PORT_REGION(0x8040DE8C, 0, 0, 0) = 0.15f / getFaderFrameRateMultiplierFloat();
    sHXTimerAsFloat                              = 0.0f;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8017E518, 0, 0, 0), adjustHXFrameRateTest4_State0);

static void adjustHXFrameRateTest4_State0R() {
    HX_SetFrameRate(-5.0f / getFaderFrameRateMultiplierFloat());
    *(f32 *)SMS_PORT_REGION(0x8040DE8C, 0, 0, 0) = -0.15f / getFaderFrameRateMultiplierFloat();
    sHXTimerAsFloat                              = 230.0f;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8017E53C, 0, 0, 0), adjustHXFrameRateTest4_State0R);

static int adjustHXTimerTest4_State0() {
    sHXTimerAsFloat += HX_GetFrameRate();
    return Max(0, static_cast<int>(sHXTimerAsFloat));
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8017E578, 0, 0, 0), adjustHXTimerTest4_State0);

HX_PATCH_TIMER(SMS_PORT_REGION(0x8017E544, 0, 0, 0), adjustHXTimerTest4_State0, 0x26);
// ----------- //

// -- TEST5 -- //
HX_PATCH_TIMER(SMS_PORT_REGION(0x8017E07C, 0, 0, 0), adjustHXTimerTest5_State0, 0x26);
// ----------- //

// HX_MotionUpdate
static float HX_MotionUpdateCustom(f32 *unk) {
    if (unk[0] <= unk[7]) {
        if (unk[1] <= unk[7]) {
            unk[6] += unk[5] / getFaderFrameRateMultiplierFloat();
        }
    } else {
        unk[6] += unk[3] / getFaderFrameRateMultiplierFloat();
    }
    unk[7] += 1 / getFaderFrameRateMultiplierFloat();
    unk[8] += unk[6] / getFaderFrameRateMultiplierFloat();
    return unk[8];
}
SMS_PATCH_B(SMS_PORT_REGION(0x80181D74, 0, 0, 0), HX_MotionUpdateCustom);

// ---------------------------------------------------------------------------

// Fade

static void adjustFaderFrameRate() {
    TSMSFader *fader;
    SMS_FROM_GPR(29, fader);

    fader->_10 = static_cast<u16>(fader->mQueuedWipeRequest.mWipeSpeed * fader->mWipeTimeCopy *
                                  getFaderFrameRateMultiplierFloat());
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8013F978, 0, 0, 0), adjustFaderFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x8013F9A0, 0, 0, 0), adjustFaderFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x8013F9D4, 0, 0, 0), adjustFaderFrameRate);
SMS_PATCH_BL(SMS_PORT_REGION(0x8013F9FC, 0, 0, 0), adjustFaderFrameRate);

///////////////
///////////////

// Shine Select
// Credits to theAzack9

///////////////
///////////////

static float getShineSelectFrameRateMultiplierFloat() {
    if (sShineSelectFrameRate == 0.0f) {
        return 1.0f;
    }

    switch (gFPSSetting.getInt()) {
    case FPSSetting::FPS_30:
        return 1.0f / sShineSelectFrameRate;
    case FPSSetting::FPS_60:
        return 2.0f / sShineSelectFrameRate;
    case FPSSetting::FPS_120:
        return 4.0f / sShineSelectFrameRate;
    }
    return 1.0f / sShineSelectFrameRate;
}

void startIncrease_TSelectShineManager_override(void *self, u32 shinesOffset) {
    const int baseFrames  = 40;
    const f32 frameScalar = getShineSelectFrameRateMultiplierFloat();

    const int newFrames = baseFrames / frameScalar;

    *(f32 *)SMS_PORT_REGION(0x80412398, 0, 0, 0) = (25.5f / frameScalar);
    *(f32 *)SMS_PORT_REGION(0x8041239C, 0, 0, 0) = (-25.5f / frameScalar);

    PowerPC::writeU32((u32 *)0x80178784, 0x1C050000 - newFrames);
    startIncrease__19TSelectShineManagerFi(self, shinesOffset);
    PowerPC::writeU32((u32 *)0x80178784, 0x1C050000 - baseFrames);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80173b4c, 0, 0, 0), startIncrease_TSelectShineManager_override);

void startDecrease_TSelectShineManager_override(void *self, u32 shinesOffset) {
    const int baseFrames  = 40;
    const f32 frameScalar = getShineSelectFrameRateMultiplierFloat();

    const int newFrames = baseFrames / frameScalar;

    *(f32 *)SMS_PORT_REGION(0x80412398, 0, 0, 0) = (25.5f / frameScalar);
    *(f32 *)SMS_PORT_REGION(0x8041239C, 0, 0, 0) = (-25.5f / frameScalar);

    PowerPC::writeU32((u32 *)0x80178680, 0x1C040000 + newFrames);
    startDecrease__19TSelectShineManagerFi(self, shinesOffset);
    PowerPC::writeU32((u32 *)0x80178680, 0x1C040000 + baseFrames);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801737ac, 0, 0, 0), startDecrease_TSelectShineManager_override);
SMS_WRITE_32(SMS_PORT_REGION(0x80178784, 0, 0, 0), 0x1c04ffec);
SMS_WRITE_32(SMS_PORT_REGION(0x80178680, 0, 0, 0), 0x1c040014);

// Text movement
static void setValue_TCoord2D_override(TCoord2D *self, s32 speed, f32 startx, f32 starty, f32 endx,
                                       f32 endy) {
    const f32 frameScalar = getShineSelectFrameRateMultiplierFloat();
    self->setValue(speed * frameScalar, startx, starty, endx, endy);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8017383c, 0, 0, 0), setValue_TCoord2D_override);
SMS_PATCH_BL(SMS_PORT_REGION(0x801738c8, 0, 0, 0), setValue_TCoord2D_override);
SMS_PATCH_BL(SMS_PORT_REGION(0x80173c54, 0, 0, 0), setValue_TCoord2D_override);
SMS_PATCH_BL(SMS_PORT_REGION(0x80173bc8, 0, 0, 0), setValue_TCoord2D_override);