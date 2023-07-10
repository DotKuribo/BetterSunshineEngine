#include <Dolphin/DVD.h>
#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <JSystem/J2D/J2DPrint.hxx>
#include <JSystem/JKernel/JKRFileLoader.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/MapObj/MapObjNormalLift.hxx>
#include <SMS/MapObj/MapObjTree.hxx>

#include <SMS/Strategic/HitActor.hxx>
#include <SMS/Strategic/LiveActor.hxx>
#include <SMS/macros.h>
#include <SMS/System/CardManager.hxx>
#include <SMS/raw_fn.hxx>

#include "module.hxx"
#include "p_settings.hxx"

static void *loadFromGlobalAndScene(const char *mdl, u32 unk_0, const char *path) {
    u32 **sdlModel =
        reinterpret_cast<u32 **>(loadModelData__16TModelDataKeeperFPCcUlPCc(mdl, unk_0, path));
    if (*sdlModel == nullptr) {
        delete sdlModel;
        sdlModel = reinterpret_cast<u32 **>(
            loadModelData__16TModelDataKeeperFPCcUlPCc(mdl, unk_0, "/common/mapobj"));
    }
    return sdlModel;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8021CD34, 0x80214C88, 0, 0), loadFromGlobalAndScene);

// // Load msound.aaf from AudioRes folder or archive (NTSC-U) [Xayrga/JoshuaMK]
static void smartMSoundLoad(u32 *data) {
    if (DVDConvertPathToEntrynum("/AudioRes/msound.aaf") < 0) {
        setParamInitDataPointer__18JAIGlobalParameterFPv(data);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80014F9C, 0x80014ff8, 0, 0), smartMSoundLoad);

/* -- PATCHES -- */

// Restore Chao Seed
SMS_WRITE_32(SMS_PORT_REGION(0x802FD1A0, 0x802f5330, 0, 0), 0x808D8C70);

// Fix Health Meter Not Rising Underwater
SMS_WRITE_32(SMS_PORT_REGION(0x801410E4, 0x80135cf8, 0, 0), 0x60000000);

// Sunscript logging restoration
//SMS_WRITE_32(SMS_PORT_REGION(0x8003DB3C, 0x8003D98C, 0, 0),
//             SMS_PORT_REGION(0x48306B08, 0x482FEE38, 0, 0));

// Show Exception Handler
SMS_WRITE_32(SMS_PORT_REGION(0x8029D0BC, 0x80294f98, 0, 0), 0x60000000);

// Extend Exception Handler
SMS_WRITE_32(SMS_PORT_REGION(0x802C7638, 0x802bf6cc, 0, 0), 0x60000000);  // gpr info
// SMS_WRITE_32(SMS_PORT_REGION(0x802C7690, 0, 0, 0), 0x60000000); // float info

#if SMS_DEBUG
// Skip FMVs
SMS_WRITE_32(SMS_PORT_REGION(0x802B5E8C, 0x802ade20, 0, 0), 0x38600001);
SMS_WRITE_32(SMS_PORT_REGION(0x802B5EF4, 0x802ade88, 0, 0), 0x38600001);
// Level Select
SMS_WRITE_32(SMS_PORT_REGION(0x802A6788, 0x8029e6e0, 0, 0), 0x3BC00009);
#endif

// Global surfing bloopies
SMS_WRITE_32(SMS_PORT_REGION(0x801B74F8, 0x801AF3B0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801B74FC, 0x801AF3B4, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801B7500, 0x801AF3B8, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801B7504, 0x801AF3BC, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801B7508, 0x801AF3C0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801B750C, 0x801AF3C4, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801B7510, 0x801AF3C8, 0, 0), 0x387E0780);
SMS_WRITE_32(SMS_PORT_REGION(0x801B7514, 0x801AF3CC, 0, 0),
             SMS_PORT_REGION(0x4810BA9D, 0x4810BCC5, 0, 0));
SMS_WRITE_32(SMS_PORT_REGION(0x801B7518, 0x801AF3D0, 0, 0), 0x28030000);
SMS_WRITE_32(SMS_PORT_REGION(0x801B751C, 0x801AF3D4, 0, 0), 0x418200A4);

// Remove save prompts
extern SavePromptsSetting gSavePromptSetting;
BETTER_SMS_FOR_CALLBACK bool conditionalSavePrompt(TMarDirector *director, u8 nextState) {
    switch (gSavePromptSetting.getInt()) {
    default:
    case SavePromptsSetting::ALL:
        return true;
    case SavePromptsSetting::NO_BLUE:
        if (nextState == 11 && reinterpret_cast<u8 *>(director)[0x261] == 1) {
            return false;
        } else {
            return true;
        }
    case SavePromptsSetting::NONE:
        if (nextState == 11 && reinterpret_cast<u8 *>(director)[0x261] != 0) {
            return false;
        } else {
            return true;
        }
    case SavePromptsSetting::AUTO_SAVE:
        if (nextState == 11 && reinterpret_cast<u8 *>(director)[0x261] != 0) {
            if (!BetterSMS::triggerAutoSave()) {
                OSReport("AutoSave failed!\n");
            }
            return false;
        } else {
            return true;
        }
    }
}