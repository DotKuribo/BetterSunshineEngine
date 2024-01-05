#include <SMS/Manager/FlagManager.hxx>
#include <SMS/Manager/ItemManager.hxx>
#include <SMS/macros.h>

#include "module.hxx"
#include <p_settings.hxx>

constexpr static const char *sShineKey =
    "\x83\x56\x83\x83\x83\x43\x83\x93\x81\x69\x82\x50\x82\x4F\x82\x4F\x96\x87\x83"
    "\x52\x83\x43\x83\x93\x97\x70\x81\x6A\x00\x00";

constexpr static const char *sShineDemoKey =
    "\x83\x56\x83\x83\x83\x43\x83\x93\x81\x69\x82\x50\x82\x4F\x82\x4F\x96\x87\x83"
    "\x52\x83\x43\x83\x93\x97\x70\x81\x6A\x83\x4A\x83\x81\x83\x89\x00\x00";

static void spawn100CoinShine(const TVec3f &position) {
    auto *nameref = TMarNameRefGen::getInstance()->getRootNameRef();

    u16 keycode = JDrama::TNameRef::calcKeyCode(sShineKey);
    if (nameref->searchF(keycode, sShineKey)) {
        gpItemManager->makeShineAppearWithDemo(sShineKey, sShineDemoKey, position.x, position.y,
                                               position.z);
    }
}

static bool sIs100ShineSpawned = false;
static size_t maybeSpawn100CoinShine(TFlagManager *manager) {
    size_t coins = manager->getFlag(0x40002);
    if (BetterSMS::areExploitsPatched()) {
        if (!sIs100ShineSpawned && manager->getFlag(0x40002) > 99) {
            spawn100CoinShine(*gpMarioPos);
            sIs100ShineSpawned = true;
        }
    } else if (manager->getFlag(0x40002) == 100) {
        spawn100CoinShine(*gpMarioPos);
    }
    return coins;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80294664, 0, 0, 0), maybeSpawn100CoinShine);
SMS_WRITE_32(SMS_PORT_REGION(0x801BED44, 0, 0, 0), 0x4800004C);

// STATIC RESETTER
BETTER_SMS_FOR_EXPORT void patches_staticResetter(TMarDirector *director) {
    sIs100ShineSpawned = TFlagManager::smInstance->getFlag(0x40002) > 99;
}