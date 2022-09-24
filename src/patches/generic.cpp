#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <JSystem/J2D/J2DPrint.hxx>
#include <JSystem/JKernel/JKRFileLoader.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/mapobj/MapObjNormalLift.hxx>
#include <SMS/mapobj/MapObjTree.hxx>

#include <SMS/actor/HitActor.hxx>
#include <SMS/actor/LiveActor.hxx>
#include <SMS/macros.h>
#include <SMS/option/CardManager.hxx>
#include <SMS/raw_fn.hxx>

#include "common_sdk.h"
#include "module.hxx"

#if BETTER_SMS_EXTRA_OBJECTS
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
#endif

// READY GO TEXT PATCH FOR THIS BULLSHIT THING DADDY NINTENDO DID
SMS_WRITE_32(SMS_PORT_REGION(0x80171C30, 0x80167A40, 0, 0), 0x2C000005);
SMS_WRITE_32(SMS_PORT_REGION(0x80171C38, 0x80167A48, 0, 0), 0x38000005);

#if BETTER_SMS_NO_TITLE_THP
// Title Screen Never Fades to THP
SMS_WRITE_32(SMS_PORT_REGION(0x8016D53C, 0x801628AC, 0, 0), 0x48000344);
#endif

// // Load msound.aaf from AudioRes folder or archive (NTSC-U) [Xayrga/JoshuaMK]
static void smartMSoundLoad(u32 *data) {
    if (*data != 0) {
        setParamInitDataPointer__18JAIGlobalParameterFPv(data);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80014F9C, 0x80014ff8, 0, 0), smartMSoundLoad);

/* -- PATCHES -- */

#if BETTER_SMS_BUGFIXES
// Restore Chao Seed
SMS_WRITE_32(SMS_PORT_REGION(0x802FD1A0, 0x802f5330, 0, 0), 0x808D8C70);

// Fix Infinte Flutter
SMS_WRITE_32(SMS_PORT_REGION(0x8028113C, 0x80278ec8, 0, 0),
             SMS_PORT_REGION(0xC002F69C, 0xC002F824, 0, 0));

// Fix Health Meter Not Rising Underwater
SMS_WRITE_32(SMS_PORT_REGION(0x801410E4, 0x80135cf8, 0, 0), 0x60000000);
#endif

#if BETTER_SMS_SPC_LOGGING
// Sunscript logging restoration
SMS_WRITE_32(SMS_PORT_REGION(0x8003DB3C, 0x8003D98C, 0, 0),
             SMS_PORT_REGION(0x48306B08, 0x482FEE38, 0, 0));
#endif

#if BETTER_SMS_EXCEPTION_HANDLER
// Show Exception Handler
SMS_WRITE_32(SMS_PORT_REGION(0x8029D0BC, 0x80294f98, 0, 0), 0x60000000);

// Extend Exception Handler
SMS_WRITE_32(SMS_PORT_REGION(0x802C7638, 0x802bf6cc, 0, 0), 0x60000000);  // gpr info
// SMS_WRITE_32(SMS_PORT_REGION(0x802C7690, 0, 0, 0), 0x60000000); // float info
#endif

#if SMS_DEBUG
// Skip FMVs
SMS_WRITE_32(SMS_PORT_REGION(0x802B5E8C, 0x802ade20, 0, 0), 0x38600001);
SMS_WRITE_32(SMS_PORT_REGION(0x802B5EF4, 0x802ade88, 0, 0), 0x38600001);
// Level Select
SMS_WRITE_32(SMS_PORT_REGION(0x802A6788, 0x8029e6e0, 0, 0), 0x3BC00009);
#endif

// Remove Dive While Wall Sliding
SMS_WRITE_32(SMS_PORT_REGION(0x8024BC10, 0x8024399c, 0, 0), 0x48000068);

// Flood Till Corona Beat
SMS_WRITE_32(SMS_PORT_REGION(0x8029961C, 0x802914b4, 0, 0), 0x38840077);

#if BETTER_SMS_LONG_JUMP
// Map on D Pad down
SMS_WRITE_32(SMS_PORT_REGION(0x8017A830, 0x801706f4, 0, 0), 0x5400077B);
SMS_WRITE_32(SMS_PORT_REGION(0x80297A60, 0x8028f8f8, 0, 0), 0x5400077B);
#endif

#if BETTER_SMS_EXTRA_OBJECTS
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
#endif

// Remove blue coin prompts
SMS_WRITE_32(SMS_PORT_REGION(0x8029A73C, 0x80292618, 0, 0), 0x60000000);

#if BETTER_SMS_UNDERWATER_FRUIT
// Fruit don't time out
SMS_WRITE_32(SMS_PORT_REGION(0x8040C918, 0x80404078, 0, 0), 0x7FFFFFFF);
#endif