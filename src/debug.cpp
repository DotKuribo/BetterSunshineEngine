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

//static void *debugNewHook(JKRHeap *heap, size_t size, int alignment) {
//    void *ptr = heap->alloc(size, alignment);
//    if (BetterSMS::isDebugMode() && heap == JKRHeap::sSystemHeap) {
//        OSReport("Allocated 0x%X bytes at block %p\n", size, ptr);
//    }
//    return ptr;
//}
//SMS_PATCH_BL(0x802C3BD8, debugNewHook);
//SMS_PATCH_BL(0x802C3C24, debugNewHook);
//SMS_PATCH_BL(0x802C3C68, debugNewHook);
//SMS_PATCH_BL(0x802C3C90, debugNewHook);
//SMS_PATCH_BL(0x802C3CD8, debugNewHook);
//SMS_PATCH_BL(0x802C3D24, debugNewHook);
//SMS_PATCH_BL(0x802C3D68, debugNewHook);
//SMS_PATCH_BL(0x802C3D90, debugNewHook);
//
//static void debugDeleteHook(JKRHeap *heap, void *block) {
//    heap->free(block);
//    if (BetterSMS::isDebugMode() && heap == JKRHeap::sSystemHeap) {
//        OSReport("Deleted block %p\n", block);
//    }
//}
//SMS_PATCH_BL(0x802C3DF0, debugDeleteHook);
//SMS_PATCH_BL(0x802C3E54, debugDeleteHook);