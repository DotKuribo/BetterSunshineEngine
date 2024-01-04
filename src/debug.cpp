#include <Dolphin/types.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>

#include <SMS/MarioUtil/DrawUtil.hxx>
#include <SMS/System/Resolution.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "debug.hxx"

#include "libs/container.hxx"
#include "libs/global_vector.hxx"
#include "libs/string.hxx"

#include "module.hxx"

using namespace BetterSMS;

static TGlobalVector<Debug::InitCallback> sDebugInitCBs;
static TGlobalVector<Debug::UpdateCallback> sDebugUpdateCBs;
static TGlobalVector<Debug::DrawCallback> sDebugDrawCBs;

BETTER_SMS_FOR_EXPORT bool BetterSMS::Debug::addInitCallback(InitCallback cb) {
    sDebugInitCBs.push_back(cb);
    return true;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Debug::addUpdateCallback(UpdateCallback cb) {
    sDebugUpdateCBs.push_back(cb);
    return true;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Debug::addDrawCallback(DrawCallback cb) {
    sDebugDrawCBs.push_back(cb);
    return true;
}

#pragma region CallbackHandlers

BETTER_SMS_FOR_CALLBACK void initDebugCallbacks(TApplication *app) {
    if (!BetterSMS::isDebugMode())
        return;

    auto *currentHeap = JKRHeap::sRootHeap->becomeCurrentHeap();

    for (auto &item : sDebugInitCBs) {
        item(app);
    }

    currentHeap->becomeCurrentHeap();
}

BETTER_SMS_FOR_CALLBACK void updateDebugCallbacks(TApplication *app) {
    if (!BetterSMS::isDebugMode())
        return;

    for (auto &item : sDebugUpdateCBs) {
        item(app);
    }
}

BETTER_SMS_FOR_CALLBACK void drawDebugCallbacks(TApplication *app, const J2DOrthoGraph *ortho) {
    if (!BetterSMS::isDebugMode())
        return;

    for (auto &item : sDebugDrawCBs) {
        item(app, ortho);
    }
}

#pragma endregion