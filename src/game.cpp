#include <SMS/System/Application.hxx>

#include "libs/container.hxx"
#include "game.hxx"
#include "module.hxx"

using namespace BetterSMS;

static TDictS<Game::BootCallback> sGameBootCBs;

SMS_NO_INLINE bool BetterSMS::Game::isOnBootRegistered(const char *name) {
    return sGameBootCBs.hasKey(name);
}

SMS_NO_INLINE bool BetterSMS::Game::registerOnBootCallback(const char *name, BootCallback cb) {
    if (sGameBootCBs.hasKey(name))
        return false;
    sGameBootCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::deregisterOnBootCallback(const char *name) {
    if (!sGameBootCBs.hasKey(name))
        return false;
    sGameBootCBs.pop(name);
    return true;
}

void gameBootCallbackHandler(TApplication *app) {
    TDictS<Game::BootCallback>::ItemList bootCBs;
    sGameBootCBs.items(bootCBs);

    for (auto &item : bootCBs) {
        item.mValue(&gpApplication);
    }
}
//SMS_PATCH_BL(SMS_PORT_REGION(0x802A66C0, 0, 0, 0), gameBootCallbackHandler);