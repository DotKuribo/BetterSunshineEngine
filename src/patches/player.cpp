#include <Dolphin/MTX.h>
#include <JSystem/JUtility/JUTColor.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/Map/Map.hxx>
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

#if 0
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
#else
    if ((carried->mObjectID & 0x18000000))
        return;

    if ((carried->mStateFlags.asU32 & 1) != 0) {
        carried->mHolder    = nullptr;
        player->mHeldObject = nullptr;
    }
#endif
}

#undef VT_COVER_FRUIT
#undef VT_RANDOM_FRUIT
#undef VT_RESET_FRUIT
#undef VT_WOOD_BARREL
#undef VT_MAP_OBJ_GENERAL
#undef VT_MAP_OBJ_BALL
#undef VT_MAP_OBJ_ITEM

#if BETTER_SMS_NO_DOWNWARP

// Remove sticky downwarp
SMS_WRITE_32(SMS_PORT_REGION(0x8018D08C, 0, 0, 0), 0x60000000);

#endif

SMS_ASM_FUNC void fixShadowPartsCrash() {
    SMS_ASM_BLOCK("cmpwi 4, 0                      \n\t"
                  "li 0, 0                         \n\t"
                  "beq 8                           \n\t"
                  "lhz 0, 0x18, (4)                \n\t"
                  "blr                             \n\t");
}
SMS_PATCH_BL(0x802320E0, fixShadowPartsCrash);

static size_t checkWallsWhenHanging(TMap *map, TBGWallCheckRecord *record) {
    return map->mCollisionData->checkWalls(record);
}
SMS_PATCH_BL(0x80260944, checkWallsWhenHanging);
SMS_PATCH_BL(0x80260AF8, checkWallsWhenHanging);
SMS_PATCH_BL(0x80260D1C, checkWallsWhenHanging);
SMS_PATCH_BL(0x80260DDC, checkWallsWhenHanging);

static f32 getHangingRoof(TMap *map, f32 x, f32 y, f32 z, const TBGCheckData **out) {
    TVec3f *pos;
    SMS_FROM_GPR(31, pos);

    if (!BetterSMS::isCollisionRepaired()) {
        return map->checkRoof(x, y, z, out);
    }

    // f32 height = 10000000.0f;
    while (true) {
        f32 result = map->checkRoof(x, y + 80.0f, z, out);
        if (result > pos->y - 80.0f) {
            return result;
        }
        y = result;
    }
}
SMS_PATCH_BL(0x80261794, getHangingRoof);