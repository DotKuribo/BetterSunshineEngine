#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DPane.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/raw_fn.hxx>

#include "debug.hxx"
#include "libs/cheathandler.hxx"
#include "libs/constmath.hxx"
#include "logging.hxx"
#include "module.hxx"

#include "p_debug.hxx"

using namespace BetterSMS;

static s16 gBaseMonitorX = 10, gMonitorY = 443;
static u16 gMonitorWidth = 210, gMonitorHeight = 6 * 3;

static f32 gSystemHeapMaxUsage, gCurrentHeapMaxUsage, gRootHeapMaxUsage;
static JKRHeap *gCurrentHeap = nullptr;

static void drawMonitorBar(f32 currentFill, f32 maxFill, JUtility::TColor color, s16 x, s16 y,
                           s16 w, s16 h) {
    J2DFillBox(x, y, w * currentFill, h, color);
    J2DFillBox(x + (w * maxFill), y, 2, h, {200, 200, 200, 255});
}

static size_t getHeapSize(JKRHeap *heap) {
    return reinterpret_cast<size_t>(heap->mEnd) - reinterpret_cast<size_t>(heap->mStart);
}

static void drawHeapUsage(JKRHeap *heap, f32 &maxUsage, JUtility::TColor color, u16 y) {
    const auto heapSize = getHeapSize(heap);

    const f32 currentUsage = static_cast<f32>((heapSize - heap->getTotalFreeSize())) / heapSize;
    if (currentUsage > maxUsage)
        maxUsage = currentUsage;

    {
        s16 adjust = getScreenRatioAdjustX();
        drawMonitorBar(currentUsage, maxUsage, color, (gBaseMonitorX - adjust) + 2, y,
                       (gMonitorWidth + adjust) - 6, 4);
    }
}

BETTER_SMS_FOR_CALLBACK void resetMonitor(TApplication *app) { gCurrentHeapMaxUsage = 0.0f; }

BETTER_SMS_FOR_CALLBACK void drawMonitor(TApplication *app, const J2DOrthoGraph *graph) {
    auto *systemHeap  = JKRHeap::sSystemHeap;
    auto *currentHeap = JKRHeap::sCurrentHeap;
    auto *rootHeap    = JKRHeap::sRootHeap;

    if (!systemHeap || !currentHeap || !rootHeap || gDebugUIPage == 0|| !BetterSMS::isDebugMode())
        return;

    if (currentHeap != gCurrentHeap) {
        gCurrentHeap         = currentHeap;
        gCurrentHeapMaxUsage = 0.0f;
    }

    {
        s16 adjust = getScreenRatioAdjustX();
        J2DFillBox(gBaseMonitorX - adjust, gMonitorY, gMonitorWidth + adjust, gMonitorHeight,
                   {0, 0, 0, 170});
    }

    drawHeapUsage(systemHeap, gSystemHeapMaxUsage, {220, 50, 30, 255}, gMonitorY + 2);
    drawHeapUsage(currentHeap, gCurrentHeapMaxUsage, {30, 230, 30, 255}, gMonitorY + 7);
    drawHeapUsage(rootHeap, gRootHeapMaxUsage, {40, 30, 230, 255}, gMonitorY + 12);
}