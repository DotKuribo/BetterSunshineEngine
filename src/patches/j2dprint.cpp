#include <Dolphin/GX.h>
#include <Dolphin/OS.h>
#include <Dolphin/string.h>

#include <JSystem/J2D/J2DPrint.hxx>
#include <JSystem/JKernel/JKRFileLoader.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/actor/Mario.hxx>
#include <SMS/mapobj/MapObjNormalLift.hxx>
#include <SMS/mapobj/MapObjTree.hxx>

#include <SMS/actor/HitActor.hxx>
#include <SMS/actor/LiveActor.hxx>
#include <SMS/macros.h>
#include <SMS/option/CardManager.hxx>
#include <SMS/raw_fn.hxx>

#include "common_sdk.h"
#include "module.hxx"

#if 0

#if BETTER_SMS_BUGFIXES

#if 0
static u8 sLineBuffer[sizeof(String) * 64];
static JKRSolidHeap sLineHeap(&sLineBuffer, 64, JKRHeap::sRootHeap, false);

static SMS_NO_INLINE size_t getSplitLines(const char *str, String **out, size_t maxLines = __UINT32_MAX__) {
  String string(str, 1024);

  size_t nlinePos = string.find('\n', 0);
  size_t nlineLast = String::npos;
  size_t numLines = 0;
  do {
    out[numLines] = new (&sLineHeap, 4) String(nlineLast+1 + nlinePos);
    string.substr(out[numLines], nlineLast+1, nlinePos == String::npos ? String::npos : nlinePos + 1);
    numLines += 1;
  } while (nlinePos != String::npos && numLines < maxLines-1);

  return numLines;
}

static int sPrintX = 0, sPrintY = 0;

static void _capturePrintPos(J2DPane *pane, int x, int y) {
  sPrintX = x;
  sPrintY = y;
  pane->makeMatrix(x, y);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802d0bec, 0, 0, 0), _capturePrintPos);

static void cullJ2DPrint(J2DPrint *printer, int unk_0, int unk_1, u8 unk_2, const char *formatter, ...) {
  constexpr int fontWidth = 20;

  va_list vargs;
  va_start(vargs, formatter);

  char *msg = va_arg(vargs, char *);

  va_end(vargs);

  if (sPrintX > 0 && (sPrintX + strlen(msg)) < SME::TGlobals::getScreenWidth())
    printer->print(0, 0, unk_2, formatter, msg);

  String finalStr(1024);

  String *lines[64];
  size_t numLines = getSplitLines(msg, lines, 64);

  for (u32 i = 0; i < numLines; ++i) {
    String *line = lines[i];
    size_t cullL = Max((-sPrintX / fontWidth) - 2, 0);
    size_t cullR = ((-sPrintX + (line->size() * fontWidth)) / SME::TGlobals::getScreenWidth()) + 2;

    line->substr(const_cast<char *>(finalStr.data()) + finalStr.end(), cullL, cullR);
  }

  printer->print(0, 0, unk_2, formatter, finalStr.data());
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802d0c20, 0, 0, 0), cullJ2DPrint);

#else

static int sPrintX = 0, sPrintY = 0;

static void _capturePrintPos(J2DPane *pane, int x, int y) {
    sPrintX = x;
    sPrintY = y;
    pane->makeMatrix(x, y);
}
// SMS_PATCH_BL(SMS_PORT_REGION(0x802d0bec, 0, 0, 0), _capturePrintPos);

static void cullJ2DPrint(J2DPrint *printer, int unk_0, int unk_1, u8 unk_2, const char *formatter,
                         ...) {
    constexpr f32 fontWidth = 16;

    va_list vargs;
    va_start(vargs, formatter);

    char *msg = va_arg(vargs, char *);

    va_end(vargs);

    if ((sPrintX > 0 && (sPrintX + strlen(msg)) < SME::TGlobals::getScreenWidth()) ||
        strchr(msg, '\n') != nullptr)
        printer->print(0, 0, unk_2, formatter, msg);

    String culledMsg(msg, 1024);

    f32 cullL = Max((f32(-sPrintX) / fontWidth) - 2.0f, 0);
    f32 cullR = Max(
        ((f32(-sPrintX) + (f32(culledMsg.size()) * fontWidth)) / SME::TGlobals::getScreenWidth()) +
            2.0f,
        cullL);

    culledMsg.substr(&culledMsg, size_t(cullL), size_t(cullR));
    culledMsg.append('\0');

    printer->print(int(cullL * fontWidth), 0, unk_2, formatter, culledMsg.data());
}
// SMS_PATCH_BL(SMS_PORT_REGION(0x802d0c20, 0, 0, 0), cullJ2DPrint);

static Mtx44 sDrawMtx;

static void captureTextboxDrawMtx(Mtx44 mtx, u8 index) {
    PSMTXCopy(mtx, sDrawMtx);
    GXLoadPosMtxImm(mtx, index);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802d0bf8, 0x802C8DA0, 0, 0), captureTextboxDrawMtx);

static void maybePrintChar(JUTFont *font, f32 x, f32 y, f32 w, f32 h, int ascii, bool unk_1) {
    const int offset =
        static_cast<int>((SME::TGlobals::getScreenToFullScreenRatio() - 1.0f) * 600.0f);
    const int fontWidth = font->getWidth();

    int absX = static_cast<int>(sDrawMtx[0][3] + x);
    int absY = static_cast<int>(sDrawMtx[1][3] + y);

    if (absX + fontWidth > -offset && absX < SME::TGlobals::getScreenWidth())
        font->drawChar_scale(x, y, w, h, ascii, unk_1);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802CEC2C, 0x802C6DD4, 0, 0), maybePrintChar);

static OSStopwatch stopwatch;
static bool sInitialized = false;
static bool sIsWaiting   = false;
static OSTick sLastStart = 0;
static void J2D_BenchMarkPrint(J2DTextBox *printer, int x, int y) {
    if (!sInitialized) {
        OSInitStopwatch(&stopwatch, "J2DPrintTest");
        sInitialized = true;
    }

    if (!sIsWaiting) {
        sLastStart = OSGetTick();
        sIsWaiting = true;
    }

    OSStartStopwatch(&stopwatch);
    printer->draw(x, y);
    OSStopStopwatch(&stopwatch);

    if (sIsWaiting && OSTicksToSeconds(OSGetTick() - sLastStart) > 5.0f) {
        OSDumpStopwatch(&stopwatch);
    }
}
// SMS_PATCH_BL(SMS_PORT_REGION(0x80144010, 0, 0, 0), J2D_BenchMarkPrint);

#endif

#endif

#endif