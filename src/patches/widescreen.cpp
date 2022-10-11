
#include <Dolphin/types.h>

#include <JSystem/JUtility/JUTRect.hxx>
#include <JSystem/JUtility/JUTTexture.hxx>
#include <SMS/G2D/ExPane.hxx>
#include <SMS/GC2D/Guide.hxx>
#include <SMS/GC2D/SelectDir.hxx>
#include <SMS/GC2D/SelectMenu.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/camera/PolarSubCamera.hxx>
#include <SMS/game/GCConsole2.hxx>
#include <SMS/game/MarDirector.hxx>
#include <SMS/macros.h>
#include <SMS/manager/SelectShineManager.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/screen/SMSFader.hxx>
#include <SMS/Player/MarioGamePad.hxx>

#include "libs/constmath.hxx"
#include "module.hxx"

using namespace BetterSMS;

#if BETTER_SMS_WIDESCREEN

static s32 getScreenTransX() { return (getScreenToFullScreenRatio() - 1.0f) * 600.0f; }

static f32 getScreenWidthTranslated() { return static_cast<f32>(getScreenWidth()) + getScreenTransX(); }
SMS_PATCH_BL(SMS_PORT_REGION(0x802A3E78, 0x8029BD54, 0, 0), getScreenWidthTranslated);
SMS_WRITE_32(SMS_PORT_REGION(0x802A3E7C, 0x8029BD58, 0, 0), 0xD03C0038);

static s32 getNegScreenTransX() { return -getScreenTransX(); }
SMS_PATCH_BL(SMS_PORT_REGION(0x80176AA4, 0x8016CA6C, 0, 0), getNegScreenTransX);
SMS_WRITE_32(SMS_PORT_REGION(0x80176AA8, 0x8016CA70, 0, 0), 0xD03B0030);
SMS_PATCH_BL(SMS_PORT_REGION(0x8029B974, 0x80293850, 0, 0), getNegScreenTransX);
SMS_WRITE_32(SMS_PORT_REGION(0x8029B978, 0x80293854, 0, 0), 0xD0350030);
SMS_PATCH_BL(SMS_PORT_REGION(0x80176C40, 0x8016CC00, 0, 0), getNegScreenTransX);
SMS_WRITE_32(SMS_PORT_REGION(0x80176C44, 0x8016CC04, 0, 0), 0xD03B0030);
SMS_PATCH_BL(SMS_PORT_REGION(0x80176FF4, 0x8016D160, 0, 0), getNegScreenTransX);
SMS_WRITE_32(SMS_PORT_REGION(0x80176FF8, 0x8016D164, 0, 0), 0xD03B0030);
SMS_PATCH_BL(SMS_PORT_REGION(0x80177198, 0x8016CFBC, 0, 0), getNegScreenTransX);
SMS_WRITE_32(SMS_PORT_REGION(0x8017719c, 0x8016CFC0, 0, 0), 0xD03B0030);

SMS_PATCH_BL(SMS_PORT_REGION(0x80176AB4, 0x8016CA7C, 0, 0), getScreenWidth);
SMS_WRITE_32(SMS_PORT_REGION(0x80176AB8, 0x8016CA80, 0, 0), 0xD03B0038);
SMS_PATCH_BL(SMS_PORT_REGION(0x8029B984, 0x80293860, 0, 0), getScreenWidth);
SMS_WRITE_32(SMS_PORT_REGION(0x8029B988, 0x80293864, 0, 0), 0xD0350038);
SMS_PATCH_BL(SMS_PORT_REGION(0x80176C50, 0x8016CC10, 0, 0), getScreenWidth);
SMS_WRITE_32(SMS_PORT_REGION(0x80176C54, 0x8016CC14, 0, 0), 0xD03B0038);
SMS_PATCH_BL(SMS_PORT_REGION(0x80177004, 0x8016D170, 0, 0), getScreenWidth);
SMS_WRITE_32(SMS_PORT_REGION(0x80177008, 0x8016D174, 0, 0), 0xD03B0038);
SMS_PATCH_BL(SMS_PORT_REGION(0x801771A8, 0x8016CFCC, 0, 0), getScreenWidth);
SMS_WRITE_32(SMS_PORT_REGION(0x801771AC, 0x8016CFD0, 0, 0), 0xD03B0038);

static f32 getScreenXRatio2() {
    const f32 ratio = getScreenToFullScreenRatio();
    return ratio + (ratio - 1.0f);
}

static f32 getShineSelectXRatio() { return getScreenXRatio2() * 1.33333337307; }

static f32 getCameraXRatio() { return getScreenXRatio2() * 0.913461446762f; }

// Shine Select Model Rot Width
SMS_PATCH_BL(SMS_PORT_REGION(0x80176E58, 0x8016CE20, 0, 0), getShineSelectXRatio);
SMS_WRITE_32(SMS_PORT_REGION(0x80176E5C, 0x8016CE24, 0, 0), 0xD03B004C);

// Camera Width
SMS_WRITE_32(SMS_PORT_REGION(0x802B8B6C, 0x802B0B3C, 0, 0), 0x90010AE4);
SMS_PATCH_BL(SMS_PORT_REGION(0x802B8B70, 0x802B0B40, 0, 0), getCameraXRatio);
SMS_WRITE_32(SMS_PORT_REGION(0x802B8B74, 0x802B0B44, 0, 0),
             SMS_PORT_REGION(0xC842FFF0, 0xC842FE70, 0, 0));
SMS_WRITE_32(SMS_PORT_REGION(0x802B8B78, 0x802B0B48, 0, 0), 0x3C80803E);
SMS_WRITE_32(SMS_PORT_REGION(0x802B8B7C, 0x802B0B4C, 0, 0), 0x3C60803E);
SMS_WRITE_32(SMS_PORT_REGION(0x802B8B88, 0x802B0B58, 0, 0), 0xC8010AE0);
SMS_WRITE_32(SMS_PORT_REGION(0x802B8B94, 0x802B0B64, 0, 0), 0xEC001028);
SMS_WRITE_32(SMS_PORT_REGION(0x802B8B9C, 0x802B0B6C, 0, 0), 0xEC010032);

#if 1

static void scaleNintendoIntro(JUTRect *rect, int x1, int y1, int x2, int y2) {
    const f32 translate = getScreenTransX();

    x1 -= translate;
    x2 += translate;

    rect->set(x1, y1, x2, y2);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80296078, 0x8028DF10, 0, 0), scaleNintendoIntro);

static void scaleGCConsoleExPane140(TExPane *pane) {
    TGCConsole2 *console;
    SMS_FROM_GPR(31, console);

    pane->mRect.mX1 -= getScreenTransX();

    reinterpret_cast<TExPane **>(console)[0x140 / 4] = pane;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8014EF74, 0x80143C04, 0, 0), scaleGCConsoleExPane140);

static void scaleGCConsoleExPane108(TExPane *pane) {
    TGCConsole2 *console;
    SMS_FROM_GPR(31, console);

    pane->mRect.mX1 -= getScreenTransX();

    reinterpret_cast<TExPane **>(console)[0x108 / 4] = pane;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8014EE24, 0x80143AB4, 0, 0), scaleGCConsoleExPane108);

static void scaleGCConsoleExPane160(TExPane *pane) {
    TGCConsole2 *console;
    SMS_FROM_GPR(31, console);

    pane->mRect.mX1 -= getScreenTransX();

    reinterpret_cast<TExPane **>(console)[0x160 / 4] = pane;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8014F09C, 0x80143D2C, 0, 0), scaleGCConsoleExPane160);

static void scaleGCConsoleExPane2F8(TExPane *pane) {
    TGCConsole2 *console;
    SMS_FROM_GPR(31, console);

    pane->mRect.mX1 += getScreenTransX();

    reinterpret_cast<TExPane **>(console)[0x2F8 / 4] = pane;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8014F308, 0x80143F98, 0, 0), scaleGCConsoleExPane2F8);

static void scaleGCConsoleExPane400(TExPane *pane) {
    TGCConsole2 *console;
    SMS_FROM_GPR(31, console);

    pane->mRect.mX1 -= getScreenTransX();

    reinterpret_cast<TExPane **>(console)[0x400 / 4] = pane;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8014F70C, 0x8014439C, 0, 0), scaleGCConsoleExPane400);

static void scaleGCConsoleExPane42C(TExPane *pane) {
    TGCConsole2 *console;
    SMS_FROM_GPR(31, console);

    pane->mRect.mX1 -= getScreenTransX();

    reinterpret_cast<TExPane **>(console)[0x42C / 4] = pane;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8014F830, 0x801444C0, 0, 0), scaleGCConsoleExPane42C);

static void scaleGCConsoleExPane450(TExPane *pane) {
    TGCConsole2 *console;
    SMS_FROM_GPR(31, console);

    pane->mRect.mX1 -= getScreenTransX();

    reinterpret_cast<TExPane **>(console)[0x450 / 4] = pane;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8014F93C, 0x801445CC, 0, 0), scaleGCConsoleExPane450);

static void scaleGCConsoleLoadAfter2F8(JUTRect *dst, const JUTRect &ref) {
    dst->copy(ref);
    dst->mX1 += getScreenTransX();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8014D8E8, 0x80142578, 0, 0), scaleGCConsoleLoadAfter2F8);

static void scaleGCConsoleLoadAfter550Add(JUTRect *src, int x, int y) {
    x += getScreenTransX();
    src->add(x, y);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8014E7E0, 0x80143470, 0, 0), scaleGCConsoleLoadAfter550Add);

static void setRecursivePanePos(JSUInputStream *stream, u16 *dst, size_t len) {
    stream->read(dst, len);

    if (*dst == 415) {
        *dst += getScreenTransX();
    } else if (*dst == 397) {
        *dst += getScreenTransX();
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802CB320, 0x802C33B4, 0, 0), setRecursivePanePos);

static void scaleGCConsoleExPane1C4(TExPane *pane) {
    TGCConsole2 *console;
    SMS_FROM_GPR(31, console);

    pane->mRect.mX1 += getScreenTransX();

    reinterpret_cast<TExPane **>(console)[0x1C4 / 4] = pane;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8014F114, 0x80143DA4, 0, 0), scaleGCConsoleExPane1C4);

static void scalePauseMenuMask(J2DScreen *screen, int x, int y, J2DGrafContext *context) {
    u32 *tPauseMenu;
    SMS_FROM_GPR(31, tPauseMenu);

    J2DPane *pane   = reinterpret_cast<J2DPane *>(tPauseMenu[0x18 / 4]);
    pane->mRect.mX1 = -getScreenTransX();
    pane->mRect.mX2 = 600 + getScreenTransX();

    screen->draw(x, y, context);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8015600C, 0x8014B028, 0, 0), scalePauseMenuMask);

static void scaleSelectMenuMask(TSelectMenu *menu) {
    TExPane *pane;

    pane = menu->mMask1;
    pane->mRect.mX1 -= getScreenTransX();
    pane->mRect.mX2 += getScreenTransX();
    pane->mPane->mRect.copy(pane->mRect);

    pane = menu->mMask2;
    pane->mRect.mX1 -= getScreenTransX();
    pane->mRect.mX2 += getScreenTransX();
    pane->mPane->mRect.copy(pane->mRect);

    startMove__11TSelectMenuFv(menu);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80175F50, 0x8016BEF4, 0, 0), scaleSelectMenuMask);

static void scaleSelectMenuGrad(u32 type, u32 idx, u32 count) {
    TSelectGrad *grad;
    SMS_FROM_GPR(31, grad);

    u8 yColorR = (grad->mColorX2.r + grad->mColorX1.r) / 2;
    u8 yColorG = (grad->mColorX2.g + grad->mColorX1.g) / 2;
    u8 yColorB = (grad->mColorX2.b + grad->mColorX1.b) / 2;

    GXBegin(type, idx, count);

    GXPosition3f32(static_cast<f32>(-getScreenTransX()), 16.0f, -100.0f);
    GXColor3u8(grad->mColorX1.r, grad->mColorX1.g, grad->mColorX1.b);

    GXPosition3f32(static_cast<f32>(getScreenWidth()), 16.0f, -100.0f);
    GXColor3u8(yColorR, yColorG, yColorB);

    GXPosition3f32(static_cast<f32>(getScreenWidth()), 464.0f, -100.0f);
    GXColor3u8(grad->mColorX2.r, grad->mColorX2.g, grad->mColorX2.b);

    GXPosition3f32(static_cast<f32>(-getScreenTransX()), 464.0f, -100.0f);
    GXColor3u8(yColorR, yColorG, yColorB);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80175868, 0x8016B80C, 0, 0), scaleSelectMenuGrad);
SMS_WRITE_32(SMS_PORT_REGION(0x8017586C, 0x8016B810, 0, 0), 0x48000090);

static u32 fixShootingStarsNoDelete() {
    u32 *emitterManager4D2 = *(u32 **)SMS_PORT_REGION(0x8040E1E4, 0x804058BC, 0, 0);
    TConsoleStr *consoleStr;
    SMS_FROM_GPR(31, consoleStr);

    for (s32 i = 2; i >= 0; --i) {
        deleteEmitter__17JPAEmitterManagerFP14JPABaseEmitter(emitterManager4D2,
                                                             consoleStr->mJPABaseEmitters[i]);
        consoleStr->mJPABaseEmitters[i] = nullptr;
    }

    return 0;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80171314, 0x800FAC3C, 0, 0), fixShootingStarsNoDelete);
SMS_WRITE_32(SMS_PORT_REGION(0x80171318, 0x800FAC40, 0, 0), 0x809F0028);

static void fixShootingStarsWideScreen(TBoundPane *pane, u32 size, JUTPoint &begin, JUTPoint &mid,
                                       JUTPoint &end) {
    const f32 ratio = getScreenToFullScreenRatio() * 1.2f;

    mid.x *= ratio;
    end.x *= ratio;

    pane->setPanePosition(size, begin, mid, end);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80170EFC, 0x80166D08, 0, 0), fixShootingStarsWideScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x80170F34, 0x80166D54, 0, 0), fixShootingStarsWideScreen);
SMS_PATCH_BL(SMS_PORT_REGION(0x80170F6C, 0x80166DA0, 0, 0), fixShootingStarsWideScreen);

static void fixDemoMasksWideScreen_InitStaticGoPanes(TConsoleStr *consoleStr) {
    loadAfter__Q26JDrama8TNameRefFv(consoleStr);

    const f32 ratio = getScreenToFullScreenRatio();

    for (u32 i = 0; i < 2; ++i) {
        JUTRect &rect = consoleStr->mDemoWipeExPanes[i]->mPane->mRect;
        rect.mX1      = static_cast<s32>(-((ratio - 1) * 600.0f));
        rect.mX2      = getScreenWidth();

        consoleStr->mDemoWipeExPanes[i]->mRect.copy(rect);
    }

    JUTRect *rect = &consoleStr->mDemoMaskExPanes[0]->mPane->mRect;
    rect->mX1     = static_cast<s32>(-((ratio - 1) * 600.0f));

    consoleStr->mDemoMaskExPanes[0]->mRect.copy(*rect);

    rect      = &consoleStr->mDemoMaskExPanes[1]->mPane->mRect;
    rect->mX2 = getScreenWidth();

    consoleStr->mDemoMaskExPanes[1]->mRect.copy(*rect);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801723F0, 0x801681E0, 0, 0),
             fixDemoMasksWideScreen_InitStaticGoPanes);

static JUTRect sGuideBorderRects[2];
static J2DPane sGuideBorderPanes[2];

static void fixGuideWideScreenOpen(TSMSFader *fader, u32 type, f32 time, f32 delay) {
    TGuide *guide;
    SMS_FROM_GPR(29, guide);

    fader->startWipe(type, time, delay);

    guide->mScreen->mRect.mX2 = 600;

    TConsoleStr *consoleStr = gpMarDirector->mGCConsole->mConsoleStr;
    const f32 ratio         = getScreenToFullScreenRatio();

    JUTRect *rect = &consoleStr->mDemoMaskExPanes[0]->mPane->mRect;
    J2DPane *pane = consoleStr->mDemoMaskExPanes[0]->mPane;

    sGuideBorderRects[0] = *rect;
    sGuideBorderPanes[0] = *pane;

    rect->mX1        = static_cast<s32>(-((ratio - 1) * 600.0f));
    rect->mX2        = guide->mScreen->mRect.mX1;
    pane->mAlpha     = 0xFF;
    pane->mIsVisible = true;

    consoleStr->mDemoMaskExPanes[0]->mRect.copy(*rect);

    rect = &consoleStr->mDemoMaskExPanes[1]->mPane->mRect;
    pane = consoleStr->mDemoMaskExPanes[1]->mPane;

    sGuideBorderRects[1] = *rect;
    sGuideBorderPanes[1] = *pane;

    rect->mX1        = guide->mScreen->mRect.mX2;
    rect->mX2        = getScreenWidth();
    pane->mAlpha     = 0xFF;
    pane->mIsVisible = true;

    consoleStr->mDemoMaskExPanes[1]->mRect.copy(*rect);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8017940C, 0x8016F2B4, 0, 0), fixGuideWideScreenOpen);

static void fixGuideWideScreenClose(TSMSFader *fader, u32 type, f32 time, f32 delay) {
    fader->startWipe(type, time, delay);

    TConsoleStr *consoleStr = gpMarDirector->mGCConsole->mConsoleStr;

    JUTRect *rect = &consoleStr->mDemoMaskExPanes[0]->mRect;
    J2DPane *pane = consoleStr->mDemoMaskExPanes[0]->mPane;

    *rect = sGuideBorderRects[0];
    *pane = sGuideBorderPanes[0];

    rect = &consoleStr->mDemoMaskExPanes[1]->mRect;
    pane = consoleStr->mDemoMaskExPanes[1]->mPane;

    *rect = sGuideBorderRects[1];
    *pane = sGuideBorderPanes[1];
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80179880, 0x8016F738, 0, 0), fixGuideWideScreenClose);

static void renderGuideWideScreenFix(J2DScreen *screen, int x, int y, J2DGrafContext *context) {
    TConsoleStr *consoleStr = gpMarDirector->mGCConsole->mConsoleStr;
    consoleStr->mOpeningDemoScreen->draw(x, y, context);

    screen->draw(x, y, context);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80179390, 0x8016F238, 0, 0), renderGuideWideScreenFix);

static void scaleGuideMap(TGuide *guide) {
    resetObjects__6TGuideFv(guide);

    J2DPane *pane = reinterpret_cast<J2DPane *>(
        reinterpret_cast<J2DPane *>(guide->mScreen->mChildrenList.mFirst->mItemPtr)
            ->mChildrenList.mFirst->mItemPtr);
    pane->mRect.mX2 = 600.0f;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8017CB64, 0x80172B94, 0, 0), scaleGuideMap);

// -- DEBS -- //

static f32 sDEBSToTimerRatio = 0.0f;
static bool sHasRedAppeared  = false;
static JUTRect sPaneRect;
static JUTRect sFillRect;

void setDEBSWidthByRedCoinTimer(TGCConsole2 *console, bool forceAppear) {
    sDEBSToTimerRatio = lerp<f32>(0.0f, 1.0f, f32(console->mRedCoinCardTimer) / 117.1f);

    TExPane *pane          = console->mTelopWindow;
    pane->mRect.mX1        = sPaneRect.mX1 + (180.0f * sDEBSToTimerRatio);
    pane->mPane->mRect.mX1 = sPaneRect.mX1 + (180.0f * sDEBSToTimerRatio);
    reinterpret_cast<J2DWindow *>(pane->mPane)->mFillRect.mX2 =
        sFillRect.mX2 - (180.0f * sDEBSToTimerRatio);

    startAppearTelop__11TGCConsole2Fb(console, forceAppear);
}

static void fixDEBSWideScreenText(s32 x1, s32 y1, s32 width, s32 height) {
    TGCConsole2 *console = gpMarDirector->mGCConsole;
    const f32 ratio      = getScreenToFullScreenRatio();

    sDEBSToTimerRatio = lerp<f32>(0.0f, 1.0f, f32(console->mRedCoinCardTimer) / 117.1f);

    TExPane *pane          = console->mTelopWindow;
    pane->mRect.mX1        = sPaneRect.mX1 + (180.0f * sDEBSToTimerRatio);
    pane->mPane->mRect.mX1 = sPaneRect.mX1 + (180.0f * sDEBSToTimerRatio);
    reinterpret_cast<J2DWindow *>(pane->mPane)->mFillRect.mX2 =
        sFillRect.mX2 - (180.0f * sDEBSToTimerRatio);

    const s32 offset = static_cast<s32>(((ratio - 1.0f) * 45.0f));
    x1 -= offset;

    GXSetScissor(x1 + (195.0f * sDEBSToTimerRatio), y1, width - (195.0f * sDEBSToTimerRatio),
                 height);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80143FDC, 0x80138CAC, 0, 0), fixDEBSWideScreenText);

static void fixDEBSWideScreenPanel(TGCConsole2 *console) {
    const f32 ratio  = getScreenToFullScreenRatio();
    const s32 offset = getScreenTransX();

    TExPane *pane = console->mTelopWindow;
    pane->mRect.mX1 -= offset;
    pane->mRect.mX2 += offset;
    pane->mPane->mRect.mX1 -= offset;
    pane->mPane->mRect.mX2 += offset;
    reinterpret_cast<J2DWindow *>(pane->mPane)->mFillRect.mX2 += offset * 2;

    sPaneRect = pane->mRect;
    sFillRect = reinterpret_cast<J2DWindow *>(pane->mPane)->mFillRect;

    sDEBSToTimerRatio          = 0.0f;
    console->mRedCoinCardTimer = 0;
    sHasRedAppeared            = false;
}

static void fixDeathScreenRatio(u32 *cardsave, TMarioGamePad *gamepad) {
    initData__9TCardSaveFP13TMarioGamePad(cardsave, gamepad);

    const s32 offset = getScreenTransX();
    J2DPane *pane    = reinterpret_cast<J2DScreen *>(cardsave[0x14 / 4])->search('mask');

    pane->mRect.mX1 -= offset;
    pane->mRect.mX2 += offset;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80163468, 0x801584B4, 0, 0), fixDeathScreenRatio);

static void fixYoshiFruitText(TGCConsole2 *console) {
    const f32 ratio  = getScreenToFullScreenRatio();
    const s32 offset = getScreenTransX();

    J2DPicture *pane = reinterpret_cast<J2DPicture **>(console)[0x314 / 4];
    pane->mRect.mX1 += offset;
    pane->mRect.mX2 += offset;
}

static void loadAfterGCConsolePatches(TGCConsole2 *console) {
    loadAfter__Q26JDrama8TNameRefFv(console);
    fixDEBSWideScreenPanel(console);
    fixYoshiFruitText(console);

    // char buffer[64];
    // char namebuf[32];

    // J2DPicture *marioIcon = reinterpret_cast<J2DPicture *>(console->mMainScreen->search('m_ic'));

    // auto *playerData = Player::getData(gpMarioAddress);
    // if (playerData && playerData->getPlayerID() == SME::Enum::Player::IL_PIANTISSIMO)
    //     snprintf(buffer, 64, "/game_6/timg/%s_icon.bti", "piantissimo");
    // else
    //     snprintf(buffer, 64, "/game_6/timg/%s_icon.bti", "mario");

    // ResTIMG *timg;
    // timg = (ResTIMG *)JKRFileLoader::getGlbResource(buffer);

    // if (timg)
    //     marioIcon->changeTexture(timg, 0);

    // J2DPicture *marioName = reinterpret_cast<J2DPicture *>(console->mMainScreen->search('m_tx'));

    // auto *playerData = Player::getData(gpMarioAddress);
    // if (playerData && playerData->getPlayerID() == SME::Enum::Player::IL_PIANTISSIMO)
    //     snprintf(buffer, 64, "/game_6/timg/%s_text.bti", "piantissimo");
    // else
    //     snprintf(buffer, 64, "/game_6/timg/%s_text.bti", "mario");
    // timg = (ResTIMG *)JKRFileLoader::getGlbResource(buffer);

    // if (timg)
    //     marioName->changeTexture(timg, 0);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8014D8A4, 0x80142534, 0, 0), loadAfterGCConsolePatches);

static void patchSMSFaderInOut(JDrama::TRect *rect, JUtility::TColor color) {
    rect->mX1 -= 1;
    rect->mX2 += 1;
    GXSetScissor(rect->mX1, rect->mY1, rect->mX2, rect->mY2);
    GXSetViewport(rect->mX1, rect->mY1, rect->mX2, rect->mY2, 0.0f, 1.0f);
    fill_rect__9(rect, color);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8013FDAC, 0x80134928, 0, 0), patchSMSFaderInOut);

static void patchLevelSelectPosition(J2DScreen *screen, int x, int y, J2DGrafContext *context) {
    reinterpret_cast<J2DPane *>(screen->mChildrenList.mFirst->mItemPtr)
        ->mRect.move(getScreenTransX(), 0);
    screen->draw(x, y, context);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8013F430, 0x80133FAC, 0, 0), patchLevelSelectPosition);

static SMS_ASM_FUNC void patchGXScissor() {
    // clang-format off
  // todo: account for screen ratio shit
    SMS_ASM_BLOCK (
        SMS_PORT_REGION (
            "lwz       7, -0x72F8(13)   \n\t",
            "lwz       7, -0x7338(13)   \n\t",
            "lwz       7, -0x72F8(13)   \n\t",
            "lwz       7, -0x72F8(13)   \n\t"
        )
        SMS_PORT_REGION (
            "lwz       0, 0x1E8(7)    \n\t",
            "lwz       0, 0x156(7)    \n\t",
            "lwz       0, 0x1E8(7)    \n\t",
            "lwz       0, 0x1E8(7)    \n\t"
        )
        "rlwinm    12,0,4,18,27       \n\t"
        "cmpwi     3, 0               \n\t"
        "beq-      .gx_loc_0x40       \n\t"
        "add       0, 3, 5            \n\t"
        "cmpw      0, 12              \n\t"
        "beq-      .gx_loc_0x40       \n\t"
        "rlwinm    0,12,31,1,31       \n\t"
        "sub       3, 3, 0            \n\t"
        "mulli     3, 3, 0x3          \n\t"
        "mulli     5, 5, 0x3          \n\t"
        "srawi     3, 3, 0x2          \n\t"
        "rlwinm    5,5,30,2,31        \n\t"
        "addze     3, 3               \n\t"
        "add       3, 3, 0            \n\t"
        ".gx_loc_0x40:                \n\t"
        "lis 12, GXSetScissor@h       \n\t"
        "ori 12, 12, GXSetScissor@l + 4       \n\t"
        "mtctr 12       \n\t"
        "bctr       \n\t"
    );
    // clang-format on
}
SMS_PATCH_B(SMS_PORT_REGION(0x80363138, 0x8035b358, 0, 0), patchGXScissor);

static void scaleUnderWaterMask(Mtx mtx, f32 x, f32 y, f32 z) {
    CPolarSubCamera *camera = gpCamera;
    x *= (reinterpret_cast<f32 *>(camera)[0x48 / 4] / 50.0f);
    PSMTXScale(mtx, x, y, z);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801ea96c, 0x801E2844, 0, 0), scaleUnderWaterMask);

#endif

#endif