#include <Dolphin/types.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <SMS/SMS.hxx>
#include <SMS/macros.h>

#include "debug.hxx"
#include "libs/container.hxx"
#include "module.hxx"

using namespace BetterSMS;

static TDictS<Debug::DebugModeInitCallback> sDebugInitCBs;
static TDictS<Debug::DebugModeUpdateCallback> sDebugUpdateCBs;
static TDictS<Debug::DebugModeDrawCallback> sDebugDrawCBs;

SMS_NO_INLINE bool BetterSMS::Debug::isDebugInitRegistered(const char *name) {
    return sDebugInitCBs.hasKey(name);
}

SMS_NO_INLINE bool BetterSMS::Debug::isDebugUpdateRegistered(const char *name) {
    return sDebugUpdateCBs.hasKey(name);
}

SMS_NO_INLINE bool BetterSMS::Debug::isDebugDrawRegistered(const char *name) {
    return sDebugDrawCBs.hasKey(name);
}

SMS_NO_INLINE bool BetterSMS::Debug::registerDebugInitCallback(const char *name,
                                                               DebugModeInitCallback cb) {
    if (sDebugInitCBs.hasKey(name))
        return false;
    sDebugInitCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Debug::registerDebugUpdateCallback(const char *name,
                                                                 DebugModeUpdateCallback cb) {
    if (sDebugUpdateCBs.hasKey(name))
        return false;
    sDebugUpdateCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Debug::registerDebugDrawCallback(const char *name,
                                                               DebugModeDrawCallback cb) {
    if (sDebugDrawCBs.hasKey(name))
        return false;
    sDebugDrawCBs.set(name, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Debug::deregisterDebugInitCallback(const char *name) {
    if (!sDebugInitCBs.hasKey(name))
        return false;
    sDebugInitCBs.pop(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Debug::deregisterDebugUpdateCallback(const char *name) {
    if (!sDebugUpdateCBs.hasKey(name))
        return false;
    sDebugUpdateCBs.pop(name);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Debug::deregisterDebugDrawCallback(const char *name) {
    if (!sDebugDrawCBs.hasKey(name))
        return false;
    sDebugDrawCBs.pop(name);
    return true;
}

#pragma region CallbackHandlers

void initDebugCallbacks(TMarDirector *director) {
    if (!director || !BetterSMS::isDebugMode())
        return;

    TDictS<Debug::DebugModeInitCallback>::ItemList initCBs;
    sDebugInitCBs.items(initCBs);

    for (auto &item : initCBs) {
        item.mItem.mValue(director);
    }
}

void updateDebugCallbacks(TMarDirector *director) {
    if (!director || !BetterSMS::isDebugMode())
        return;

    TDictS<Debug::DebugModeUpdateCallback>::ItemList updateCBs;
    sDebugUpdateCBs.items(updateCBs);

    for (auto &item : updateCBs) {
        item.mItem.mValue(director);
    }
}

void drawDebugCallbacks(TMarDirector *director, J2DOrthoGraph *ortho) {
    if (!director || !ortho || !BetterSMS::isDebugMode())
        return;

    TDictS<Debug::DebugModeDrawCallback>::ItemList drawCBs;
    sDebugDrawCBs.items(drawCBs);

    for (auto &item : drawCBs) {
        item.mItem.mValue(director, ortho);
    }
}

#pragma endregion