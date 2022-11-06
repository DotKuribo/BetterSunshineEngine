#include <SMS/Player/Mario.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/Player/Watergun.hxx>
#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>


#include "module.hxx"

#if BETTER_SMS_HOVER_SLIDE

using namespace BetterSMS;

static void checkHoverSlideFOV(CPolarSubCamera *camera, int mode, int sub, bool unk_1) {
    camera->changeCamModeSub_(mode, sub, unk_1);
}
// SME_PATCH_BL(SME_PORT_REGION(0x80021af8, 0, 0, 0), checkHoverSlideFOV);

#endif