#include <Dolphin/types.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>

#include <SMS/MarioUtil/DrawUtil.hxx>
#include <SMS/System/Resolution.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "debug.hxx"

#include "libs/global_unordered_map.hxx"
#include "libs/container.hxx"
#include "libs/string.hxx"

#include "module.hxx"

using namespace BetterSMS;

static TGlobalUnorderedMap<TGlobalString, Debug::InitCallback> sDebugInitCBs(32);
static TGlobalUnorderedMap<TGlobalString, Debug::UpdateCallback> sDebugUpdateCBs(32);
static TGlobalUnorderedMap<TGlobalString, Debug::DrawCallback> sDebugDrawCBs(32);

SMS_NO_INLINE bool BetterSMS::Debug::registerInitCallback(const char *name, InitCallback cb) {
    if (sDebugInitCBs.find(name) != sDebugInitCBs.end())
        return false;
    sDebugInitCBs[name] = cb;
    return true;
}

SMS_NO_INLINE bool BetterSMS::Debug::registerUpdateCallback(const char *name, UpdateCallback cb) {
    if (sDebugUpdateCBs.find(name) != sDebugUpdateCBs.end())
        return false;
    sDebugUpdateCBs[name] = cb;
    return true;
}

SMS_NO_INLINE bool BetterSMS::Debug::registerDrawCallback(const char *name, DrawCallback cb) {
    if (sDebugDrawCBs.find(name) != sDebugDrawCBs.end())
        return false;
    sDebugDrawCBs[name] = cb;
    return true;
}

SMS_NO_INLINE void BetterSMS::Debug::deregisterInitCallback(const char *name) {
    sDebugInitCBs.erase(name);
}

SMS_NO_INLINE void BetterSMS::Debug::deregisterUpdateCallback(const char *name) {
    sDebugUpdateCBs.erase(name);
}

SMS_NO_INLINE void BetterSMS::Debug::deregisterDrawCallback(const char *name) {
    sDebugDrawCBs.erase(name);
}

#pragma region CallbackHandlers

void initDebugCallbacks(TApplication *app) {
    if (!BetterSMS::isDebugMode())
        return;

    auto *currentHeap = JKRHeap::sRootHeap->becomeCurrentHeap();

    for (auto &item : sDebugInitCBs) {
        item.second(app);
    }

    currentHeap->becomeCurrentHeap();
}

static bool sIsActive = true;

void updateDebugCallbacks(TApplication *app) {
    if (!BetterSMS::isDebugMode())
        return;

    if ((app->mGamePads[0]->mButtons.mInput & TMarioGamePad::L) &&
        (app->mGamePads[0]->mButtons.mFrameInput & TMarioGamePad::DPAD_UP))
        sIsActive ^= true;

    if (!sIsActive)
        return;

    for (auto &item : sDebugUpdateCBs) {
        item.second(app);
    }
}

void drawDebugCallbacks(TApplication *app, const J2DOrthoGraph *ortho) {
    if (!BetterSMS::isDebugMode())
        return;

    if (!sIsActive)
        return;

    for (auto &item : sDebugDrawCBs) {
        item.second(app, ortho);
    }
}

#pragma endregion