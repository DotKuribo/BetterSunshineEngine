#include <SMS/System/Application.hxx>

#include <SMS/MarioUtil/DrawUtil.hxx>
#include <SMS/MarioUtil/gd-reinit-gx.hxx>
#include <SMS/System/Resolution.hxx>
#include <SMS/raw_fn.hxx>

#include "libs/container.hxx"
#include "game.hxx"
#include "module.hxx"

using namespace BetterSMS;

static TDictS<Game::InitCallback> sGameInitCBs;
static TDictS<Game::BootCallback> sGameBootCBs;
static TDictS<Game::LoopCallback> sGameLoopCBs;
static TDictS<Game::DrawCallback> sGameDrawCBs;
static TDictS<Game::ChangeCallback> sGameChangeCBs;

SMS_NO_INLINE bool BetterSMS::Game::isOnInitRegistered(const char *name) {
    return sGameInitCBs.hasKey(name);
}

SMS_NO_INLINE bool BetterSMS::Game::isOnBootRegistered(const char *name) {
    return sGameBootCBs.hasKey(name);
}

SMS_NO_INLINE bool BetterSMS::Game::isOnLoopRegistered(const char *name) {
    return sGameLoopCBs.hasKey(name);
}

SMS_NO_INLINE bool BetterSMS::Game::isOnPostDrawRegistered(const char *name) {
    return sGameDrawCBs.hasKey(name);
}

SMS_NO_INLINE bool BetterSMS::Game::isOnChangeRegistered(const char *name) {
    return sGameChangeCBs.hasKey(name);
}

SMS_NO_INLINE bool BetterSMS::Game::registerOnInitCallback(const char *name, InitCallback cb) {
    if (sGameInitCBs.hasKey(name))
        return false;
    sGameInitCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::registerOnBootCallback(const char *name, BootCallback cb) {
    if (sGameBootCBs.hasKey(name))
        return false;
    sGameBootCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::registerOnLoopCallback(const char *name, LoopCallback cb) {
    if (sGameLoopCBs.hasKey(name))
        return false;
    sGameLoopCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::registerOnPostDrawCallback(const char *name, DrawCallback cb) {
    if (sGameDrawCBs.hasKey(name))
        return false;
    sGameDrawCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::registerOnChangeCallback(const char *name, ChangeCallback cb) {
    if (sGameChangeCBs.hasKey(name))
        return false;
    sGameChangeCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::deregisterOnInitCallback(const char *name) {
    if (!sGameInitCBs.hasKey(name))
        return false;
    sGameInitCBs.pop(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::deregisterOnBootCallback(const char *name) {
    if (!sGameBootCBs.hasKey(name))
        return false;
    sGameBootCBs.pop(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::deregisterOnLoopCallback(const char *name) {
    if (!sGameLoopCBs.hasKey(name))
        return false;
    sGameLoopCBs.pop(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::deregisterOnPostDrawCallback(const char *name) {
    if (!sGameDrawCBs.hasKey(name))
        return false;
    sGameDrawCBs.pop(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::deregisterOnChangeCallback(const char *name) {
    if (!sGameChangeCBs.hasKey(name))
        return false;
    sGameChangeCBs.pop(name);
    return true;
}

// extern -> custom app proc
void gameInitCallbackHandler(TApplication *app) {
    TDictS<Game::InitCallback>::ItemList initCBs;
    sGameInitCBs.items(initCBs);

    for (auto &item : initCBs) {
        item.mValue(&gpApplication);
    }
}

// extern -> custom app proc
void gameBootCallbackHandler(TApplication *app) {
    TDictS<Game::BootCallback>::ItemList bootCBs;
    sGameBootCBs.items(bootCBs);

    for (auto &item : bootCBs) {
        item.mValue(&gpApplication);
    }
}

extern void updateStageCallbacks(TApplication *);
extern void updateDebugCallbacks(TApplication *);
extern void drawLoadingScreen(TApplication *, const J2DOrthoGraph *);
extern void drawDebugCallbacks(TApplication *, const J2DOrthoGraph *);

// extern -> custom app proc
s32 gameLoopCallbackHandler(JDrama::TDirector *director) {
    TDictS<Game::LoopCallback>::ItemList loopCBs;
    sGameLoopCBs.items(loopCBs);

    for (auto &item : loopCBs) {
        item.mValue(&gpApplication);
    }

    updateStageCallbacks(&gpApplication);
    updateDebugCallbacks(&gpApplication);

    s32 ret = director->direct();

    /*drawLoadingScreen(&gpApplication);
    drawDebugCallbacks(&gpApplication);*/

    return ret;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802A616C, 0x8029E07C, 0, 0), gameLoopCallbackHandler);

void gameDrawCallbackHandler() {
    ReInitializeGX();

    J2DOrthoGraph ortho(0, 0, BetterSMS::getScreenOrthoWidth(), 448);
    ortho.setup2D();

    GXSetViewport(0, 0, 640, 480, 0, 1);
    {
        Mtx44 mtx;
        C_MTXOrtho(mtx, 16, 496, -BetterSMS::getScreenRatioAdjustX(),
                   BetterSMS::getScreenRenderWidth(), -1, 1);
        GXSetProjection(mtx, GX_ORTHOGRAPHIC);
    }

    {
        TDictS<Game::DrawCallback>::ItemList drawCBs;
        sGameDrawCBs.items(drawCBs);

        for (auto &item : drawCBs) {
            item.mValue(&gpApplication, &ortho);
        }
    }

    drawLoadingScreen(&gpApplication, &ortho);
    drawDebugCallbacks(&gpApplication, &ortho);

    THPPlayerDrawDone();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802A630C, 0, 0, 0), gameDrawCallbackHandler);

extern void exitStageCallbacks(TApplication *);

// extern -> custom app proc
void gameChangeCallbackHandler(TApplication *app) {
    TDictS<Game::ChangeCallback>::ItemList changeCBs;
    sGameChangeCBs.items(changeCBs);

    for (auto &item : changeCBs) {
        item.mValue(&gpApplication);
    }

    exitStageCallbacks(app);
}