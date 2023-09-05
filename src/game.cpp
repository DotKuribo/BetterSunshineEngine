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

static size_t sMaxShines = 120;
static TGlobalUnorderedMap<TGlobalString, Game::InitCallback> sGameInitCBs(32);
static TGlobalUnorderedMap<TGlobalString, Game::BootCallback> sGameBootCBs(32);
static TGlobalUnorderedMap<TGlobalString, Game::LoopCallback> sGameLoopCBs(32);
static TGlobalUnorderedMap<TGlobalString, Game::DrawCallback> sGameDrawCBs(32);
static TGlobalUnorderedMap<TGlobalString, Game::ChangeCallback> sGameChangeCBs(32);

BETTER_SMS_FOR_EXPORT size_t BetterSMS::Game::getMaxShines() {
    return sMaxShines;
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Game::setMaxShines(size_t maxShines) {
    sMaxShines = maxShines;
}

// Register a function to be called on game init
BETTER_SMS_FOR_EXPORT bool BetterSMS::Game::registerInitCallback(const char *name,
                                                                 InitCallback cb) {
    if (sGameInitCBs.find(name) != sGameInitCBs.end())
        return false;
    sGameInitCBs[name] = cb;
    return true;
}

// Register a function to be called on game boot
BETTER_SMS_FOR_EXPORT bool BetterSMS::Game::registerBootCallback(const char *name,
                                                                 BootCallback cb) {
    if (sGameBootCBs.find(name) != sGameBootCBs.end())
        return false;
    sGameBootCBs[name] = cb;
    return true;
}

// Register a function to be called every game loop
BETTER_SMS_FOR_EXPORT bool BetterSMS::Game::registerLoopCallback(const char *name,
                                                                 LoopCallback cb) {
    if (sGameLoopCBs.find(name) != sGameLoopCBs.end())
        return false;
    sGameLoopCBs[name] = cb;
    return true;
}

// Register a function to be called after the game draws graphics
BETTER_SMS_FOR_EXPORT bool BetterSMS::Game::registerPostDrawCallback(const char *name,
                                                                     DrawCallback cb) {
    if (sGameDrawCBs.find(name) != sGameDrawCBs.end())
        return false;
    sGameDrawCBs[name] = cb;
    return true;
}

// Register a function to be called on game context change
BETTER_SMS_FOR_EXPORT bool BetterSMS::Game::registerChangeCallback(const char *name,
                                                                   ChangeCallback cb) {
    if (sGameChangeCBs.find(name) != sGameChangeCBs.end())
        return false;
    sGameChangeCBs[name] = cb;
    return true;
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Game::deregisterInitCallback(const char *name) {
    sGameInitCBs.erase(name);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Game::deregisterBootCallback(const char *name) {
    sGameBootCBs.erase(name);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Game::deregisterLoopCallback(const char *name) {
    sGameLoopCBs.erase(name);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Game::deregisterPostDrawCallback(const char *name) {
    sGameDrawCBs.erase(name);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Game::deregisterChangeCallback(const char *name) {
    sGameChangeCBs.erase(name);
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