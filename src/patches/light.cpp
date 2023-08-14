#include "module.hxx"
#include "settings.hxx"

static void safeLightVectorNormalized(const TVec3f &src, TVec3f &dst) {
    if (BetterSMS::getBugFixesSetting() && src.x == 0.0f && src.y == 0.0f && src.z == 0.0f) {
        dst = src;
        return;
    }
    PSVECNormalize(src, dst);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80231148, 0, 0, 0), safeLightVectorNormalized);