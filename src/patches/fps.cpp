#include <Dolphin/types.h>

#include <JSystem/JDrama/JDRDirector.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>

#include <SMS/GC2D/Talk2D2.hxx>
#include <SMS/System/MarDirector.hxx>
#include <SMS/raw_fn.hxx>

#include "p_settings.hxx"
#include "libs/constmath.hxx"

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

BETTER_SMS_FOR_CALLBACK void updateFPS(TApplication *app) {
    const bool wantsForcedVSync = app->mContext == TApplication::CONTEXT_DIRECT_LOAD_LOOP ||
                                  app->mContext == TApplication::CONTEXT_DIRECT_MAIN_LOOP ||
                                  app->mContext == TApplication::CONTEXT_GAME_BOOT ||
                                  app->mContext == TApplication::CONTEXT_GAME_BOOT_LOGO ||
                                  app->mContext == TApplication::CONTEXT_GAME_INTRO;

    if (wantsForcedVSync) {
        gpApplication.mDisplay->mRetraceCount                                   = 2;
        *reinterpret_cast<f32 *>(SMS_PORT_REGION(0x804167B8, 0x8040DD10, 0, 0)) = 0.5f;
        *reinterpret_cast<f32 *>(SMS_PORT_REGION(0x80414904, 0x8040BE54, 0, 0)) = 0.01f;
        return;
    }

    switch (gFPSSetting.getInt()) {
    case FPSSetting::FPS_30:
        gpApplication.mDisplay->mRetraceCount                                   = 2;
        *reinterpret_cast<f32 *>(SMS_PORT_REGION(0x804167B8, 0x8040DD10, 0, 0)) = 0.5f;
        *reinterpret_cast<f32 *>(SMS_PORT_REGION(0x80414904, 0x8040BE54, 0, 0)) = 0.01f;
        break;
    case FPSSetting::FPS_60:
        gpApplication.mDisplay->mRetraceCount = 1;
        *reinterpret_cast<f32 *>(SMS_PORT_REGION(0x804167B8, 0x8040DD10, 0, 0)) = 1.0f;
        *reinterpret_cast<f32 *>(SMS_PORT_REGION(0x80414904, 0x8040BE54, 0, 0)) = 0.02f;
        break;
    case FPSSetting::FPS_120:
        gpApplication.mDisplay->mRetraceCount = 0;
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
static void getJointCoinAnmFrameRateCharacter(J3DFrameCtrl *frameCtrl) {
    //if (gpMarDirector->mAreaID != TGameSequence::AREA_MAREUNDERSEA) {
    //    frameCtrl->mFrameRate = 2.0f * 0.25f;
    //    return;
    //}

    f32 scalar;
    switch (gFPSSetting.getInt()) {
    case FPSSetting::FPS_30:
        scalar = 2.0f;
        break;
    case FPSSetting::FPS_60:
        scalar = 1.65f;
        break;
    case FPSSetting::FPS_120:
        scalar = 1.3f;
        break;
    }
    frameCtrl->mFrameRate = scalar * 0.25f;
}

static void getJointCoinAnmFrameRateMovement(J3DFrameCtrl *frameCtrl) {
    // This is the coin fish
    if (true || gpMarDirector->mAreaID == TGameSequence::AREA_MAREUNDERSEA) {
        frameCtrl->mFrameRate = 2.0f * 0.25f;
        return;
    }

    //f32 scalar;
    //switch (gFPSSetting.getInt()) { case FPSSetting::FPS_30:
    //    scalar = 2.0f;
    //    break;
    //case FPSSetting::FPS_60:
    //    scalar = 2.5f;
    //    break;
    //case FPSSetting::FPS_120:
    //    scalar = 3.0f;
    //    break;
    //}
    //frameCtrl->mFrameRate = scalar * 0.25f;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801F76C0, 0, 0, 0), getJointCoinAnmFrameRateCharacter);
SMS_PATCH_BL(SMS_PORT_REGION(0x801F76DC, 0, 0, 0), getJointCoinAnmFrameRateMovement);

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

static inline void HX_SetGameOverDelta(f32 frameRate) {
    *(f32 *)SMS_PORT_REGION(0x8040DE58, 0, 0, 0) = frameRate;
}

static inline void HX_SetGameOverTimer(f32 frameRate) {
    *(u32 *)SMS_PORT_REGION(0x8040DE54, 0, 0, 0) = static_cast<u32>(frameRate);
}

static inline f32 HX_GetFrameRate() { return *(f32 *)SMS_PORT_REGION(0x8040DE90, 0, 0, 0); }

static float getFaderFrameRateMultiplierFloat() {
    if (sFaderFrameRate == 0.0f) {
        return 1.0f;
    }

    const bool wantsForcedVSync =
        gpApplication.mContext == TApplication::CONTEXT_DIRECT_LOAD_LOOP ||
        gpApplication.mContext == TApplication::CONTEXT_DIRECT_MAIN_LOOP ||
        gpApplication.mContext == TApplication::CONTEXT_GAME_BOOT ||
        gpApplication.mContext == TApplication::CONTEXT_GAME_BOOT_LOGO ||
        gpApplication.mContext == TApplication::CONTEXT_GAME_INTRO;

    if (wantsForcedVSync) {
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

static float getGameOverFrameRateMultiplierFloat() {
    const bool wantsForcedVSync =
        gpApplication.mContext == TApplication::CONTEXT_DIRECT_LOAD_LOOP ||
        gpApplication.mContext == TApplication::CONTEXT_DIRECT_MAIN_LOOP ||
        gpApplication.mContext == TApplication::CONTEXT_GAME_BOOT ||
        gpApplication.mContext == TApplication::CONTEXT_GAME_BOOT_LOGO ||
        gpApplication.mContext == TApplication::CONTEXT_GAME_INTRO;

    if (wantsForcedVSync) {
        return 1.0f;
    }

    switch (gFPSSetting.getInt()) {
    case FPSSetting::FPS_30:
        return 1.0f;
    case FPSSetting::FPS_60:
        return 2.0f;
    case FPSSetting::FPS_120:
        return 4.0f;
    }
    return 1.0f;
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

#define HX_PATCH_TIMER_FOR_GAMEOVER(address, name, frames)                                                      \
    HX_PATCH(address, SMS_CONCATENATE(_tm_, name), HX_SetTimer, frames * getGameOverFrameRateMultiplierFloat())
// clang-format on

// -- CIRCLE -- //
HX_PATCH_TIMER(SMS_PORT_REGION(0x80181B54, 0, 0, 0), adjustHXTimerCircle_State0, 0x19);
HX_PATCH_TIMER(SMS_PORT_REGION(0x80181B78, 0, 0, 0), adjustHXTimerCircle_State1, 0x1E);
// ----------- //
//
// -- GAMEOVER -- //
HX_PATCH_TIMER_FOR_GAMEOVER(SMS_PORT_REGION(0x801804B0, 0, 0, 0), adjustHXTimerGameOver_State0,
                            0x32);
HX_PATCH_TIMER_FOR_GAMEOVER(SMS_PORT_REGION(0x801804E8, 0, 0, 0), adjustHXTimerGameOver_State1,
                            0xA);
HX_PATCH_TIMER_FOR_GAMEOVER(SMS_PORT_REGION(0x80180610, 0, 0, 0), adjustHXTimerGameOver_State4,
                            0x64);

static f32 adjustHXTimerGameOverMag_State1(u32 ret) {
    return 0.074f / getGameOverFrameRateMultiplierFloat();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801804EC, 0, 0, 0), adjustHXTimerGameOverMag_State1);

static void adjustHXTimerGameOverFade_State1(u32 ret) {
    *((f32 *)SMS_PORT_REGION(0x8040DE4C, 0, 0, 0)) +=
        5.0999999f / getGameOverFrameRateMultiplierFloat();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80180514, 0, 0, 0), adjustHXTimerGameOverFade_State1);

static void adjustHXTimerGameOverMag_State2(u32 ret) {
    *((f32 *)SMS_PORT_REGION(0x8040C6BC, 0, 0, 0)) -= 0.1f / getGameOverFrameRateMultiplierFloat();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8018055C, 0, 0, 0), adjustHXTimerGameOverMag_State2);

static void adjustHXTimerGameOverAlpha_State4(u32 ret) {
    *((u8 *)SMS_PORT_REGION(0x8040DE5C, 0, 0, 0)) +=
        8 / static_cast<u8>(getGameOverFrameRateMultiplierFloat());
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80180628, 0, 0, 0), adjustHXTimerGameOverAlpha_State4);

const f32 *__HX_GameOverDeltaTable = (const f32 *)SMS_PORT_REGION(0x803C12E8, 0, 0, 0);
const s32 *__HX_GameOverState      = (const s32 *)SMS_PORT_REGION(0x8040DE50, 0, 0, 0);

static void adjustHXTimerGameOverDelta_State2(u32 ret) {
    const f32 startDelta = __HX_GameOverDeltaTable[0];
    HX_SetGameOverDelta(startDelta / getGameOverFrameRateMultiplierFloat());
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80180538, 0, 0, 0), adjustHXTimerGameOverDelta_State2);

static u32 adjustHXTimerGameOverDelta_State3(u32 ret, f32 delta) {
    HX_SetGameOverDelta(delta / getGameOverFrameRateMultiplierFloat());
    return ret;
}
SMS_WRITE_32(SMS_PORT_REGION(0x8018059C, 0, 0, 0), 0xC0240070);
SMS_WRITE_32(SMS_PORT_REGION(0x801805A8, 0, 0, 0), 0x7C7D0214);
SMS_WRITE_32(SMS_PORT_REGION(0x801805AC, 0, 0, 0), 0x908D9C90);
SMS_PATCH_BL(SMS_PORT_REGION(0x801805B0, 0, 0, 0), adjustHXTimerGameOverDelta_State3);

static void adjustHXTimerGameOverTimer_State3(u32 ret) {
    HX_SetGameOverTimer(ret * getGameOverFrameRateMultiplierFloat());
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801805BC, 0, 0, 0), adjustHXTimerGameOverTimer_State3);

SMS_WRITE_32(SMS_PORT_REGION(0x801805E0, 0, 0, 0), 0x980D9C9C);
HX_PATCH_TIMER(SMS_PORT_REGION(0x801805E4, 0, 0, 0), adjustHXTimerGameOver_State3, 0x20);

static float HX_MotionUpdateGameOver(f32 *unk) {
    if (unk[0] <= unk[7]) {
        if (unk[1] <= unk[7]) {
            unk[6] += unk[5] / getGameOverFrameRateMultiplierFloat();
        }
    } else {
        unk[6] += unk[3] / getGameOverFrameRateMultiplierFloat();
    }
    unk[7] += 1 / getGameOverFrameRateMultiplierFloat();
    unk[8] += unk[6] / getGameOverFrameRateMultiplierFloat();
    return unk[8];
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80180500, 0, 0, 0), HX_MotionUpdateGameOver);
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

static void trickFaderDelayToBeCorrect(TSMSFader *fader, u32 kind, f32 speed, f32 delay) {
    fader->mQueuedWipeRequest.mWipeRequest = kind;
    fader->mQueuedWipeRequest.mWipeSpeed   = speed;
    fader->mQueuedWipeRequest.mDelayTime =
        delay *
        (kind == 0xD ? getGameOverFrameRateMultiplierFloat() : getFaderFrameRateMultiplierFloat());
}
SMS_PATCH_B(SMS_PORT_REGION(0x8013f860, 0, 0, 0), trickFaderDelayToBeCorrect);

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

 static f32 QFSync() {
    const bool wantsForcedVSync = (gpApplication.mContext == TApplication::CONTEXT_DIRECT_STAGE && gpApplication.mDirector &&
     ((TMarDirector *)gpApplication.mDirector)->mCurState == TMarDirector::STATE_INTRO_INIT);
    return wantsForcedVSync ? 30.0f : SMSGetVSyncTimesPerSec();
}
 SMS_PATCH_BL(SMS_PORT_REGION(0x80299850, 0, 0, 0), QFSync);

 static void getCurrentPullParamsNormalized(TMario* player, f32* x, f32* z) {
     player->getCurrentPullParams(x, z);
     x[0] *= (30.0f / getFrameRate());
     z[0] *= (30.0f / getFrameRate());
 }
 //SMS_PATCH_BL(SMS_PORT_REGION(0x8025EAA8, 0, 0, 0), getCurrentPullParamsNormalized);

 static SMS_ASM_FUNC void correctPerformFiltersForFireWanwanTailHit(void *tailhit, u32 flags, JDrama::TGraphics *graphics, TVec3f *x, TVec3f *z) {
     //if ((flags & 0x1001) == 0x1001) {
     //    return true;
     //}
     //return false;

     SMS_ASM_BLOCK("andi. 4, 4, 0x3001\r\n"
                   "xori 4, 4, 0x1\r\n"
                   "cntlzw 4, 4\r\n"
                   "srwi 4, 4, 5\r\n"
                   "cmpwi 4, 0\r\n"
                   "blr\r\n");
 }
 SMS_PATCH_BL(SMS_PORT_REGION(0x8008D11C, 0, 0, 0), correctPerformFiltersForFireWanwanTailHit);
 SMS_WRITE_32(SMS_PORT_REGION(0x8008D0E8, 0, 0, 0), 0x9421FEE0);
 SMS_WRITE_32(SMS_PORT_REGION(0x8008D0EC, 0, 0, 0), 0xBF6100DC);
 SMS_WRITE_32(SMS_PORT_REGION(0x8008D108, 0, 0, 0), 0x3be40000);
 SMS_WRITE_32(SMS_PORT_REGION(0x8008D10C, 0, 0, 0), 0x3b630000);
 SMS_WRITE_32(SMS_PORT_REGION(0x8008D110, 0, 0, 0), 0x3b850000);
 SMS_WRITE_32(SMS_PORT_REGION(0x8008D114, 0, 0, 0), 0x3ba60000);
 SMS_WRITE_32(SMS_PORT_REGION(0x8008D118, 0, 0, 0), 0x3bc70000);

 static void normalizePeteyThrowUpFrameRate(TLiveActor* pakkun, u32 animId) {
     changeBck__11TBossPakkunFi(pakkun, animId);
     pakkun->mActorData->setFrameRate(SMSGetAnmFrameRate() * 0.8f, 0);
 }
 SMS_PATCH_BL(SMS_PORT_REGION(0x800932BC, 0, 0, 0), normalizePeteyThrowUpFrameRate);