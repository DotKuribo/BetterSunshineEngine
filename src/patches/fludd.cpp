#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <JSystem/J2D/J2DPrint.hxx>
#include <JSystem/JKernel/JKRFileLoader.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/actor/Mario.hxx>
#include <SMS/mapobj/MapObjNormalLift.hxx>
#include <SMS/mapobj/MapObjTree.hxx>

#include <SMS/actor/HitActor.hxx>
#include <SMS/actor/LiveActor.hxx>
#include <SMS/macros.h>
#include <SMS/option/CardManager.hxx>

#include "common_sdk.h"
#include "geometry.hxx"
#include "libs/constmath.hxx"
#include "module.hxx"

#if BETTER_SMS_BUGFIXES

using namespace BetterSMS;
using namespace BetterSMS::Geometry;

static SMS_ASM_FUNC bool makeWaterHitCheckForDeath(TBGCheckData *col) {
    // clang-format off
  SMS_ASM_BLOCK (
    "lhz 0, 0 (3)             \n\t"
    "cmpwi 0, 2048            \n\t"
    "bne .makeWaterCheck_tmp0 \n\t"
    "li 0, 1025               \n\t"
    ".makeWaterCheck_tmp0:    \n\t"
    SMS_PORT_REGION (
      "lis 12, 0x8018           \n\t"
      "ori 12, 12, 0xC36C       \n\t",

      "lis 12, 0x8018           \n\t"
      "ori 12, 12, 0x4bf4       \n\t",

      "lis 12, 0           \n\t"
      "ori 12, 12, 0       \n\t",

      "lis 12, 0           \n\t"
      "ori 12, 12, 0       \n\t"
    )
    "mtctr 12                 \n\t"
    "bctr                     \n\t"
  );
    // clang-format on
}
SMS_PATCH_B(SMS_PORT_REGION(0x8018C368, 0x80184BF0, 0, 0), makeWaterHitCheckForDeath);

static void normalizeHoverSlopeSpeed(f32 floorPos) {
    TMario *player;
    SMS_FROM_GPR(22, player);

    player->mPosition.y = floorPos;

    if (!(player->mState == static_cast<u32>(TMario::STATE_HOVER)))
        return;

    const f32 playerRotY    = f32(player->mAngle.y) / 182.0f;
    const Vec playerForward = {sinf(angleToRadians(-playerRotY)), 0.0f,
                               cosf(angleToRadians(playerRotY))};
    const Vec up            = {0.0f, 1.0f, 0.0f};

    Vec floorNormal;
    PSVECNormalize(reinterpret_cast<Vec *>(player->mFloorTriangle->getNormal()), &floorNormal);

    const f32 slopeStrength = PSVECDotProduct(&up, &floorNormal);
    if (slopeStrength < 0.0f)
        return;

    const f32 lookAtRatio = Vector3::lookAtRatio(playerForward, floorNormal);

    player->mForwardSpeed =
        Min(player->mForwardSpeed,
            10.0f * clamp(scaleLinearAtAnchor(slopeStrength, lookAtRatio, 1.0f), 0.0f, 1.0f));
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802568F0, 0x8024E67C, 0, 0), normalizeHoverSlopeSpeed);

#endif