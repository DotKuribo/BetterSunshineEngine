#include <Dolphin/GX.h>
#include <Dolphin/GX_types.h>

#include <SMS/raw_fn.hxx>

#include "p_settings.hxx"
#include "module.hxx"

#if 0
static u8 s_samplePattern[12][2] = {
    {3, 5},
    {7, 3},
    {5, 7},
    {3, 5},
    {7, 3},
    {5, 7},
    {3, 5},
    {7, 3},
    {5, 7},
    {3, 5},
    {7, 3},
    {5, 7},
};

static void injectGraphicsFilterConfigurations(bool useAA, u8 samplePattern[12][2], bool doVertFilt,
                                               u8 vFilt[7]) {
    useAA |= true; // Setting for AA
    
    if (!samplePattern) {
        samplePattern = useAA ? s_samplePattern : nullptr;
    }

    GXSetCopyFilter(useAA, samplePattern, doVertFilt, vFilt);
}
SMS_PATCH_BL(0x802F9090, injectGraphicsFilterConfigurations);
#endif

extern GammaSetting gGammaSetting;

static bool sGammaTHPFlag = false;

static bool flagGammaTHPStart(bool ret) {
    if (sGammaTHPFlag) {
        return ret;
    }
    *((u8 *)gpApplication.mDisplay + 0x42) = 0;
    *((u8 *)gpApplication.mDisplay + 0x43) = 0;
    return ret;
}
SMS_PATCH_B(0x8001F050, flagGammaTHPStart);

static u32 flagGammaTHPEnd() {
    const f32 gamma             = gGammaSetting.getFloat();
    *((u8 *)gpApplication.mDisplay + 0x42) = static_cast<u8>(8.0001f * gamma);
    *((u8 *)gpApplication.mDisplay + 0x43) = static_cast<u8>(8.0001f * gamma);
    return 0x803F0000;
}
SMS_WRITE_32(0x8001EF2C, 0x90010004);
SMS_PATCH_BL(0x8001EF30, flagGammaTHPEnd);

BETTER_SMS_FOR_CALLBACK void resetGammaSetting(TApplication *director) {
    sGammaTHPFlag                          = false;
    const f32 gamma                        = gGammaSetting.getFloat();
    *((u8 *)gpApplication.mDisplay + 0x42) = static_cast<u8>(8.0001f * gamma);
    *((u8 *)gpApplication.mDisplay + 0x43) = static_cast<u8>(8.0001f * gamma);
}

BETTER_SMS_FOR_CALLBACK void updateGammaSetting(TMarDirector *director) {
    const f32 gamma = gGammaSetting.getFloat();
    if (director->mCurState != TMarDirector::STATE_INTRO_INIT) {
        sGammaTHPFlag                          = true;
        *((u8 *)gpApplication.mDisplay + 0x42) = static_cast<u8>(8.0001f * gamma);
        *((u8 *)gpApplication.mDisplay + 0x43) = static_cast<u8>(8.0001f * gamma);
    }
}
