#include <Dolphin/GX.h>
#include <Dolphin/OS.h>
#include <Dolphin/types.h>
#include <JSystem/JGeometry.hxx>
#include <JSystem/JUtility/JUTTexture.hxx>

#include <SMS/System/Application.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "libs/anim2d.hxx"
#include "icons.hxx"
#include "loading.hxx"
#include "module.hxx"

// clang-format off

static u8 sLoadingScreenBloFull[] = {
  0x53, 0x43, 0x52, 0x4e, 0x62, 0x6c, 0x6f, 0x31, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x06, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x49, 0x4e, 0x46, 0x31, 0x00, 0x00, 0x00, 0x10, 0x02, 0x58, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 
  0x50, 0x41, 0x4e, 0x31, 0x00, 0x00, 0x00, 0x18, 0x06, 0x01, 0x00, 0x00, 0x52, 0x4f, 0x4f, 0x54, 
  0x00, 0x00, 0x00, 0x00, 0x02, 0x58, 0x01, 0xe0, 0x42, 0x47, 0x4e, 0x31, 0x00, 0x00, 0x00, 0x08, 
  0x50, 0x49, 0x43, 0x31, 0x00, 0x00, 0x00, 0x20, 0x06, 0x01, 0x00, 0x00, 0x69, 0x63, 0x6f, 0x6e, 
  0x02, 0x30, 0x01, 0x90, 0x00, 0x20, 0x00, 0x20, 0x05, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x05, 
  0x45, 0x4e, 0x44, 0x31, 0x00, 0x00, 0x00, 0x08, 0x45, 0x58, 0x54, 0x31, 0x00, 0x00, 0x00, 0x08
};

static u8 sLoadingScreenBloWide[] = {
  0x53, 0x43, 0x52, 0x4e, 0x62, 0x6c, 0x6f, 0x31, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x06, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x49, 0x4e, 0x46, 0x31, 0x00, 0x00, 0x00, 0x10, 0x02, 0xe4, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 
  0x50, 0x41, 0x4e, 0x31, 0x00, 0x00, 0x00, 0x18, 0x06, 0x01, 0x00, 0x00, 0x52, 0x4f, 0x4f, 0x54, 
  0xff, 0xce, 0x00, 0x00, 0x02, 0xe4, 0x01, 0xe0, 0x42, 0x47, 0x4e, 0x31, 0x00, 0x00, 0x00, 0x08, 
  0x50, 0x49, 0x43, 0x31, 0x00, 0x00, 0x00, 0x20, 0x06, 0x01, 0x00, 0x00, 0x69, 0x63, 0x6f, 0x6e, 
  0x02, 0x88, 0x01, 0x90, 0x00, 0x20, 0x00, 0x20, 0x05, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x05, 
  0x45, 0x4e, 0x44, 0x31, 0x00, 0x00, 0x00, 0x08, 0x45, 0x58, 0x54, 0x31, 0x00, 0x00, 0x00, 0x08
};

// clang-format on

using namespace BetterSMS;

static bool sIsLoading = false;

static const u8 *sLoadingIconTIMGs[] = {
    gShineSpriteIconFrame1,
    gShineSpriteIconFrame2,
    gShineSpriteIconFrame3,
    gShineSpriteIconFrame4,
    gShineSpriteIconFrame5,
    gShineSpriteIconFrame6,
    gShineSpriteIconFrame7,
    gShineSpriteIconFrame8,
    gShineSpriteIconFrame9,
    gShineSpriteIconFrame10,
    gShineSpriteIconFrame11,
    gShineSpriteIconFrame12,
    gShineSpriteIconFrame13,
    gShineSpriteIconFrame14,
    gShineSpriteIconFrame15,
    gShineSpriteIconFrame16};

static SimpleTexAnimator sLoadingIconAnimator(sLoadingIconTIMGs, 16);
static J2DScreen *sLoadingScreenFull;
static J2DScreen *sLoadingScreenWide;
static JUTTexture sLoadingIconTexture;

void Loading::setLoading(bool isLoading) {
    if (!sIsLoading && isLoading)
        sLoadingIconAnimator.resetAnimation();
    sIsLoading = isLoading;
}

void Loading::setLoadingIconB(const ResTIMG **textures, size_t texCount) {
    sLoadingIconAnimator.setTextures(textures, texCount);
}
void Loading::setLoadingIconW(const ResTIMG **textures, size_t texCount) {
    sLoadingIconAnimator.setTextures(textures, texCount);
}
void Loading::setFullScreenLayout(J2DScreen *screen) { sLoadingScreenFull = screen; }
void Loading::setWideScreenLayout(J2DScreen *screen) { sLoadingScreenFull = screen; }
void Loading::setFrameRate(f32 fps) { sLoadingIconAnimator.setFrameRate(fps); }

#pragma region Implementation

static int baseIconX = 400;

void initLoadingScreen() {
    {
        JSUMemoryInputStream stream(sLoadingScreenBloFull, sizeof(sLoadingScreenBloFull));
        sLoadingScreenFull = new J2DScreen();
        sLoadingScreenFull->makeHiearachyPanes(sLoadingScreenFull, &stream, false, true, false,
                                               nullptr);
    }

    {
        JSUMemoryInputStream stream(sLoadingScreenBloWide, sizeof(sLoadingScreenBloWide));
        sLoadingScreenWide = new J2DScreen();
        sLoadingScreenWide->makeHiearachyPanes(sLoadingScreenWide, &stream, false, true, false,
                                               nullptr);
    }

    sLoadingIconTexture.mTexObj2.val[2] = 0;
    sLoadingIconTexture.storeTIMG(GetResourceTextureHeader(sLoadingIconTIMGs[0]));
    sLoadingIconTexture._50 = false;

    {
        J2DPicture *loadingIcon =
            reinterpret_cast<J2DPicture *>(sLoadingScreenFull->search('icon'));
        if (loadingIcon) {
            JUTRect rect = loadingIcon->mRect;
            loadingIcon->insert(&sLoadingIconTexture, 0, 1.0);
            loadingIcon->mRect = rect;
        }
    }

    {
        J2DPicture *loadingIcon =
            reinterpret_cast<J2DPicture *>(sLoadingScreenWide->search('icon'));
        if (loadingIcon) {
            JUTRect rect = loadingIcon->mRect;
            loadingIcon->insert(&sLoadingIconTexture, 0, 1.0);
            loadingIcon->mRect = rect;
        }
    }

    sLoadingIconAnimator.setFrameRate(16.0f);
}

void drawLoadingScreen(TApplication *app) {
    if (!sIsLoading || !sLoadingScreenFull || !sLoadingScreenWide)
        return;

    const f32 screenRatio = BetterSMS::getScreenToFullScreenRatio();

    auto *screen = screenRatio > 1.1f ? sLoadingScreenWide : sLoadingScreenFull;

    auto loadingIcon = screen->search('icon');
    if (loadingIcon) {
        sLoadingIconAnimator.process(reinterpret_cast<J2DPicture *>(loadingIcon));
    }

    J2DOrthoGraph ortho(screen->mRect);
    screen->draw(0, 0, &ortho);
}

#pragma endregion