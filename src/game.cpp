#include <SMS/System/Application.hxx>

#include <SMS/MarioUtil/DrawUtil.hxx>
#include <SMS/MarioUtil/gd-reinit-gx.hxx>
#include <SMS/System/Resolution.hxx>
#include <SMS/raw_fn.hxx>

#include "libs/container.hxx"
#include "libs/global_unordered_map.hxx"
#include "libs/string.hxx"

#include "game.hxx"
#include "module.hxx"

using namespace BetterSMS;

static TGlobalUnorderedMap<TGlobalString, Game::InitCallback> sGameInitCBs(32);
static TGlobalUnorderedMap<TGlobalString, Game::BootCallback> sGameBootCBs(32);
static TGlobalUnorderedMap<TGlobalString, Game::LoopCallback> sGameLoopCBs(32);
static TGlobalUnorderedMap<TGlobalString, Game::DrawCallback> sGameDrawCBs(32);
static TGlobalUnorderedMap<TGlobalString, Game::ChangeCallback> sGameChangeCBs(32);

SMS_NO_INLINE bool BetterSMS::Game::isOnInitRegistered(const char *name) {
    return sGameInitCBs.contains(name);
}

SMS_NO_INLINE bool BetterSMS::Game::isOnBootRegistered(const char *name) {
    return sGameBootCBs.contains(name);
}

SMS_NO_INLINE bool BetterSMS::Game::isOnLoopRegistered(const char *name) {
    return sGameLoopCBs.contains(name);
}

SMS_NO_INLINE bool BetterSMS::Game::isOnPostDrawRegistered(const char *name) {
    return sGameDrawCBs.contains(name);
}

SMS_NO_INLINE bool BetterSMS::Game::isOnChangeRegistered(const char *name) {
    return sGameChangeCBs.contains(name);
}

SMS_NO_INLINE bool BetterSMS::Game::registerOnInitCallback(const char *name, InitCallback cb) {
    if (isOnInitRegistered(name))
        return false;
    sGameInitCBs[name] = cb;
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::registerOnBootCallback(const char *name, BootCallback cb) {
    if (isOnBootRegistered(name))
        return false;
    sGameBootCBs[name] = cb;
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::registerOnLoopCallback(const char *name, LoopCallback cb) {
    if (isOnLoopRegistered(name))
        return false;
    sGameLoopCBs[name] = cb;
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::registerOnPostDrawCallback(const char *name, DrawCallback cb) {
    if (isOnPostDrawRegistered(name))
        return false;
    sGameDrawCBs[name] = cb;
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::registerOnChangeCallback(const char *name, ChangeCallback cb) {
    if (isOnChangeRegistered(name))
        return false;
    sGameChangeCBs[name] = cb;
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::deregisterOnInitCallback(const char *name) {
    if (!isOnInitRegistered(name))
        return false;
    sGameInitCBs.erase(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::deregisterOnBootCallback(const char *name) {
    if (!isOnBootRegistered(name))
        return false;
    sGameBootCBs.erase(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::deregisterOnLoopCallback(const char *name) {
    if (!isOnLoopRegistered(name))
        return false;
    sGameLoopCBs.erase(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::deregisterOnPostDrawCallback(const char *name) {
    if (!isOnPostDrawRegistered(name))
        return false;
    sGameDrawCBs.erase(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Game::deregisterOnChangeCallback(const char *name) {
    if (!isOnChangeRegistered(name))
        return false;
    sGameChangeCBs.erase(name);
    return true;
}

// extern -> custom app proc
void gameInitCallbackHandler(TApplication *app) {
    for (auto &item : sGameInitCBs) {
        item.second(&gpApplication);
    }
}

// extern -> custom app proc
void gameBootCallbackHandler(TApplication *app) {
    for (auto &item : sGameBootCBs) {
        item.second(&gpApplication);
    }
}

extern void updateStageCallbacks(TApplication *);
extern void updateDebugCallbacks(TApplication *);
extern void drawLoadingScreen(TApplication *, const J2DOrthoGraph *);
extern void drawDebugCallbacks(TApplication *, const J2DOrthoGraph *);

// extern -> custom app proc
s32 gameLoopCallbackHandler(JDrama::TDirector *director) {
    for (auto &item : sGameLoopCBs) {
        item.second(&gpApplication);
    }

    updateStageCallbacks(&gpApplication);
    updateDebugCallbacks(&gpApplication);

    s32 ret = director->direct();

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
        for (auto &item : sGameDrawCBs) {
            item.second(&gpApplication, &ortho);
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
    for (auto &item : sGameChangeCBs) {
        item.second(&gpApplication);
    }

    exitStageCallbacks(app);
}