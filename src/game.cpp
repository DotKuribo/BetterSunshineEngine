#include <SMS/System/Application.hxx>

#include <SMS/MarioUtil/DrawUtil.hxx>
#include <SMS/MarioUtil/gd-reinit-gx.hxx>
#include <SMS/System/Resolution.hxx>
#include <SMS/raw_fn.hxx>

#include "libs/container.hxx"
#include "libs/global_vector.hxx"
#include "libs/string.hxx"

#include "game.hxx"
#include "module.hxx"

using namespace BetterSMS;

static size_t sMaxShines = 120;
static TGlobalVector<Game::InitCallback> sGameInitCBs;
static TGlobalVector<Game::BootCallback> sGameBootCBs;
static TGlobalVector<Game::LoopCallback> sGameLoopCBs;
static TGlobalVector<Game::DrawCallback> sGameDrawCBs;
static TGlobalVector<Game::ChangeCallback> sGameChangeCBs;

BETTER_SMS_FOR_EXPORT size_t BetterSMS::Game::getMaxShines() { return sMaxShines; }

BETTER_SMS_FOR_EXPORT void BetterSMS::Game::setMaxShines(size_t maxShines) {
    sMaxShines = maxShines;
    PowerPC::writeU32(reinterpret_cast<u32 *>(SMS_PORT_REGION(0x8016725C, 0, 0, 0)),
                      0x28000000 | maxShines);
}

// Register a function to be called on game init
BETTER_SMS_FOR_EXPORT bool BetterSMS::Game::addInitCallback(InitCallback cb) {
    sGameInitCBs.push_back(cb);
    return true;
}

// Register a function to be called on game boot
BETTER_SMS_FOR_EXPORT bool BetterSMS::Game::addBootCallback(BootCallback cb) {
    sGameBootCBs.push_back(cb);
    return true;
}

// Register a function to be called every game loop
BETTER_SMS_FOR_EXPORT bool BetterSMS::Game::addLoopCallback(LoopCallback cb) {
    sGameLoopCBs.push_back(cb);
    return true;
}

// Register a function to be called after the game draws graphics
BETTER_SMS_FOR_EXPORT bool BetterSMS::Game::addPostDrawCallback(DrawCallback cb) {
    sGameDrawCBs.push_back(cb);
    return true;
}

// Register a function to be called on game context change
BETTER_SMS_FOR_EXPORT bool BetterSMS::Game::addChangeCallback(ChangeCallback cb) {
    sGameChangeCBs.push_back(cb);
    return true;
}

// extern -> custom app proc
void gameInitCallbackHandler(TApplication *app) {
    for (auto &item : sGameInitCBs) {
        item(&gpApplication);
    }
}

// extern -> custom app proc
void gameBootCallbackHandler(TApplication *app) {
    for (auto &item : sGameBootCBs) {
        item(&gpApplication);
    }
}

extern void updateStageCallbacks(TApplication *);
extern void updateDebugCallbacks(TApplication *);
extern void drawLoadingScreen(TApplication *, const J2DOrthoGraph *);
extern void drawDebugCallbacks(TApplication *, const J2DOrthoGraph *);

// extern -> custom app proc
s32 gameLoopCallbackHandler(JDrama::TDirector *director) {
    for (auto &item : sGameLoopCBs) {
        item(&gpApplication);
    }

    updateStageCallbacks(&gpApplication);
    updateDebugCallbacks(&gpApplication);

    s32 ret = director->direct();

    return ret;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802A616C, 0x8029E07C, 0, 0), gameLoopCallbackHandler);

void gameDrawCallbackHandler() {
    THPPlayerDrawDone();
    {
        J2DOrthoGraph ortho(0, 0, BetterSMS::getScreenOrthoWidth(), 448);
        ortho.setup2D();

        GXSetViewport(0, 0, 640, 480, 0, 1);
        {
            Mtx44 mtx;
            C_MTXOrtho(mtx, 16, 496, -BetterSMS::getScreenRatioAdjustX(),
                       600.0f + BetterSMS::getScreenRatioAdjustX(), -1, 1);
            GXSetProjection(mtx, GX_ORTHOGRAPHIC);
        }

        drawLoadingScreen(&gpApplication, &ortho);
        drawDebugCallbacks(&gpApplication, &ortho);

        {
            for (auto &item : sGameDrawCBs) {
                item(&gpApplication, &ortho);
            }
        }
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802a630c, 0, 0, 0), gameDrawCallbackHandler);

extern void exitStageCallbacks(TApplication *);

// extern -> custom app proc
void gameChangeCallbackHandler(TApplication *app) {
    for (auto &item : sGameChangeCBs) {
        item(&gpApplication);
    }

    exitStageCallbacks(app);
}