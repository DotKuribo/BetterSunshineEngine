#include <SMS/macros.h>
#include <SMS/Manager/FlagManager.hxx>


#include "module.hxx"

#if BETTER_SMS_BUGFIXES

static bool sIs100ShineSpawned = false;
static bool is100CoinShine(TFlagManager *manager, u32 id) {
    if (!sIs100ShineSpawned && manager->getFlag(id) > 99) {
        sIs100ShineSpawned = true;
        return true;
    }
    return false;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BED3C, 0x801B6BF4, 0, 0), is100CoinShine);
SMS_WRITE_32(SMS_PORT_REGION(0x801BED40, 0x801B6BF8, 0, 0), 0x2C030001);

// STATIC RESETTER
void patches_staticResetter() { sIs100ShineSpawned = false; }

#endif