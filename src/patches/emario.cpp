#include <Dolphin/MTX.h>
#include <JSystem/JUtility/JUTColor.hxx>
#include <SMS/enemy/EnemyMario.hxx>
#include <SMS/actor/HitActor.hxx>
#include <SMS/actor/LiveActor.hxx>
#include <SMS/macros.h>
#include <SMS/option/CardManager.hxx>
#include <SMS/raw_fn.hxx>


#include "common_sdk.h"
#include "libs/constmath.hxx"
#include "module.hxx"

using namespace BetterSMS;

#if BETTER_SMS_SHADOW_MARIO_HEALTHBAR

// ------------ //
// Shadow Mario //
// ------------ //

static JUtility::TColor getEMarioHealthBarRGBA(TEnemyMario *eMario) {
    JUtility::TColor color;
    const s16 maxHealth = ((s16 *)eMario->mEnemyManager)[0x40 / 2];

    color.set(0xFF, 0x00, 0x00, 0xFF);
    color.g =
        lerp<u8>(0, 255, static_cast<float>(eMario->mEnemyHealth) / static_cast<float>(maxHealth));
    color.r -= color.g;

    return color;
}

static void manageEMarioHealthWrapper(TEnemyMario *eMario, Mtx *posMtx) {
    *(JUtility::TColor *)0x8040FA90 = getEMarioHealthBarRGBA(eMario);
    drawHPMeter__11TEnemyMarioFPA4_f(eMario, posMtx);
}
SMS_WRITE_32(SMS_PORT_REGION(0x8003FD94, 0x8003FBE4, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x8003FDAC, 0x8003FBFC, 0, 0), manageEMarioHealthWrapper);

#endif

// Upsize Shadow Mario's hitbox to be the same as Mario
SMS_WRITE_32(SMS_PORT_REGION(0x8040FAA4, 0x80407188, 0, 0), 0x42A00000);
SMS_WRITE_32(SMS_PORT_REGION(0x8040FAA8, 0x8040718c, 0, 0), 0x42480000);