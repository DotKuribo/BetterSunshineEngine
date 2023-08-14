#include <Dolphin/MTX.h>
#include <JSystem/JUtility/JUTColor.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Strategic/HitActor.hxx>
#include <SMS/Strategic/LiveActor.hxx>
#include <SMS/System/CardManager.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "libs/constmath.hxx"
#include "module.hxx"
#include "p_settings.hxx"

using namespace BetterSMS;

#define VT_COVER_FRUIT     SMS_PORT_REGION(0x803D2850, 0, 0, 0)
#define VT_RANDOM_FRUIT    SMS_PORT_REGION(0x803D29B4, 0, 0, 0)
#define VT_RESET_FRUIT     SMS_PORT_REGION(0x803D2BA4, 0, 0, 0)
#define VT_WOOD_BARREL     SMS_PORT_REGION(0x803C28d8, 0, 0, 0)
#define VT_MAP_OBJ_GENERAL SMS_PORT_REGION(0x803c8b20, 0, 0, 0)
#define VT_MAP_OBJ_BALL    SMS_PORT_REGION(0x803d2d94, 0, 0, 0)
#define VT_MAP_OBJ_ITEM    SMS_PORT_REGION(0x803ca344, 0, 0, 0)

void checkForForceDropOnDeadActor(TMario *player, bool isMario) {
    auto *carried = reinterpret_cast<TLiveActor *>(player->mHeldObject);
    if (!carried || !BetterSMS::areBugsPatched())
        return;

    switch (*(u32 *)carried) {
    case VT_COVER_FRUIT:
    case VT_RANDOM_FRUIT:
    case VT_RESET_FRUIT:
    case VT_WOOD_BARREL:
    case VT_MAP_OBJ_GENERAL:
    case VT_MAP_OBJ_BALL:
    case VT_MAP_OBJ_ITEM:
        if ((carried->mStateFlags.asU32 & 1) != 0) {
            carried->mHolder    = nullptr;
            player->mHeldObject = nullptr;
        }
    default:
        return;
    }
}

#undef VT_COVER_FRUIT
#undef VT_RANDOM_FRUIT
#undef VT_RESET_FRUIT
#undef VT_WOOD_BARREL
#undef VT_MAP_OBJ_GENERAL
#undef VT_MAP_OBJ_BALL
#undef VT_MAP_OBJ_ITEM