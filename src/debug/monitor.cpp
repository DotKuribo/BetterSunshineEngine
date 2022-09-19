#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DPane.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <SMS/enemy/EnemyMario.hxx>
#include <SMS/SMS.hxx>
#include <SMS/actor/Mario.hxx>
#include <SMS/npc/BaseNPC.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/sound/MSound.hxx>
#include <SMS/sound/MSoundSESystem.hxx>

#include "debug.hxx"
#include "libs/cheathandler.hxx"
#include "libs/constmath.hxx"
#include "logging.hxx"
#include "module.hxx"

using namespace BetterSMS;

#if BETTER_SMS_WIDESCREEN
static s16 gMonitorX = -86, gMonitorY = 443;
static u16 gMonitorWidth = 300, gMonitorHeight = 6 * 3;
#else
static s16 gMonitorX = 10, gMonitorY = 443;
static u16 gMonitorWidth = 210, gMonitorHeight = 6 * 3;
#endif

static f32 gSystemHeapMaxUsage, gCurrentHeapMaxUsage, gRootHeapMaxUsage;
static JKRHeap *gCurrentHeap = nullptr;

static void drawMonitorBar(f32 currentFill, f32 maxFill, JUtility::TColor color, u16 x, u16 y,
                           u16 w, u16 h) {
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

    drawMonitorBar(currentUsage, maxUsage, color, gMonitorX + 2, y, gMonitorWidth - 6, 4);
}

void resetMonitor(TApplication *director) { gCurrentHeapMaxUsage = 0.0f; }

void drawMonitor(TMarDirector *director, J2DOrthoGraph *graph) {
    auto *systemHeap  = JKRHeap::sSystemHeap;
    auto *currentHeap = JKRHeap::sCurrentHeap;
    auto *rootHeap    = JKRHeap::sRootHeap;

    if (!systemHeap || !currentHeap || !rootHeap)
        return;

    if (currentHeap != gCurrentHeap) {
        gCurrentHeap         = currentHeap;
        gCurrentHeapMaxUsage = 0.0f;
    }

    J2DFillBox(gMonitorX, gMonitorY, gMonitorWidth, gMonitorHeight, {0, 0, 0, 170});

    drawHeapUsage(systemHeap, gSystemHeapMaxUsage, {220, 50, 30, 255}, gMonitorY + 2);
    drawHeapUsage(currentHeap, gCurrentHeapMaxUsage, {30, 230, 30, 255}, gMonitorY + 7);
    drawHeapUsage(rootHeap, gRootHeapMaxUsage, {40, 30, 230, 255}, gMonitorY + 12);
}