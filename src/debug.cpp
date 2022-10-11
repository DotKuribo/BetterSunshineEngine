#include <Dolphin/types.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>

#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "debug.hxx"
#include "libs/container.hxx"
#include "module.hxx"

using namespace BetterSMS;

static TDictS<Debug::InitCallback> sDebugInitCBs;
static TDictS<Debug::UpdateCallback> sDebugUpdateCBs;
static TDictS<Debug::DrawCallback> sDebugDrawCBs;

SMS_NO_INLINE bool BetterSMS::Debug::isInitRegistered(const char *name) {
    return sDebugInitCBs.hasKey(name);
}

SMS_NO_INLINE bool BetterSMS::Debug::isUpdateRegistered(const char *name) {
    return sDebugUpdateCBs.hasKey(name);
}

SMS_NO_INLINE bool BetterSMS::Debug::isDrawRegistered(const char *name) {
    return sDebugDrawCBs.hasKey(name);
}

SMS_NO_INLINE bool BetterSMS::Debug::registerInitCallback(const char *name, InitCallback cb) {
    if (sDebugInitCBs.hasKey(name))
        return false;
    sDebugInitCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Debug::registerUpdateCallback(const char *name, UpdateCallback cb) {
    if (sDebugUpdateCBs.hasKey(name))
        return false;
    sDebugUpdateCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Debug::registerDrawCallback(const char *name, DrawCallback cb) {
    if (sDebugDrawCBs.hasKey(name))
        return false;
    sDebugDrawCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Debug::deregisterInitCallback(const char *name) {
    if (!sDebugInitCBs.hasKey(name))
        return false;
    sDebugInitCBs.pop(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Debug::deregisterUpdateCallback(const char *name) {
    if (!sDebugUpdateCBs.hasKey(name))
        return false;
    sDebugUpdateCBs.pop(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Debug::deregisterDrawCallback(const char *name) {
    if (!sDebugDrawCBs.hasKey(name))
        return false;
    sDebugDrawCBs.pop(name);
    return true;
}

#pragma region CallbackHandlers

void initDebugCallbacks(TApplication *app) {
    if (!BetterSMS::isDebugMode())
        return;

    TDictS<Debug::InitCallback>::ItemList initCBs;
    sDebugInitCBs.items(initCBs);

    auto *currentHeap = JKRHeap::sRootHeap->becomeCurrentHeap();

    for (auto &item : initCBs) {
        item.mValue(app);
    }

    currentHeap->becomeCurrentHeap();
}

void updateDebugCallbacks() {
    if (!BetterSMS::isDebugMode())
        return;

    TDictS<Debug::UpdateCallback>::ItemList updateCBs;
    sDebugUpdateCBs.items(updateCBs);

    for (auto &item : updateCBs) {
        item.mValue(&gpApplication);
    }
}

extern void drawLoadingScreen(TApplication *app);

void drawDebugCallbacks(J2DOrthoGraph *ortho) {
    THPPlayerDrawDone();

    // Big hack, doing this saves resources even if polluting the file
    drawLoadingScreen(&gpApplication);

    if (!BetterSMS::isDebugMode())
        return;

    // J2DOrthoGraph ortho(0, 0, BetterSMS::getScreenWidth(), 480);

    TDictS<Debug::DrawCallback>::ItemList drawCBs;
    sDebugDrawCBs.items(drawCBs);

    for (auto &item : drawCBs) {
        item.mValue(&gpApplication, ortho);
    }
}
// SMS_PATCH_BL(SMS_PORT_REGION(0x802A630C, 0, 0, 0), drawDebugCallbacks);

#pragma endregion