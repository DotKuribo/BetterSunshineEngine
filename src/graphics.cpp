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

BETTER_SMS_FOR_CALLBACK void updateGammaSetting(TApplication *app) {
    f32 gamma = gGammaSetting.getFloat();
    if (THPPlayerGetState() != 0) {
        *((u8 *)app->mDisplay + 0x42) = 0;
        *((u8 *)app->mDisplay + 0x43) = 0;
    } else {
        *((u8 *)app->mDisplay + 0x42) = static_cast<u8>(8.0f * gamma);
        *((u8 *)app->mDisplay + 0x43) = static_cast<u8>(8.0f * gamma);
    }
}