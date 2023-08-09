#include <Dolphin/GX.h>
#include <Dolphin/OS.h>
#include <Dolphin/types.h>
#include <JSystem/JGeometry/JGMVec.hxx>
#include <JSystem/JUtility/JUTTexture.hxx>

#include <SMS/System/Application.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "libs/anim2d.hxx"
#include "loading.hxx"
#include "module.hxx"
#include "p_icons.hxx"
#include "p_settings.hxx"

// clang-format off

static u8 SMS_ALIGN(32) sLoadingScreenBlo[] = {
  0x53, 0x43, 0x52, 0x4e, 0x62, 0x6c, 0x6f, 0x31, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x06, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x49, 0x4e, 0x46, 0x31, 0x00, 0x00, 0x00, 0x10, 0x02, 0x58, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 
  0x50, 0x41, 0x4e, 0x31, 0x00, 0x00, 0x00, 0x18, 0x06, 0x01, 0x00, 0x00, 0x52, 0x4f, 0x4f, 0x54, 
  0x00, 0x00, 0x00, 0x00, 0x02, 0x58, 0x01, 0xe0, 0x42, 0x47, 0x4e, 0x31, 0x00, 0x00, 0x00, 0x08, 
  0x50, 0x49, 0x43, 0x31, 0x00, 0x00, 0x00, 0x20, 0x06, 0x01, 0x00, 0x00, 0x69, 0x63, 0x6f, 0x6e, 
  0x02, 0x18, 0x01, 0x90, 0x00, 0x20, 0x00, 0x20, 0x05, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x05, 
  0x45, 0x4e, 0x44, 0x31, 0x00, 0x00, 0x00, 0x08, 0x45, 0x58, 0x54, 0x31, 0x00, 0x00, 0x00, 0x08
};

// clang-format on

using namespace BetterSMS;

static bool sIsLoading = false;

static SimpleTexAnimator sLoadingIconAnimator(sLoadingIconTIMGs, 16);
static J2DScreen *sLoadingScreen;
static JUTTexture sLoadingIconTexture;

void Loading::setLoading(bool isLoading) {
    if (!sIsLoading && isLoading)
        sLoadingIconAnimator.resetAnimation();
    sIsLoading = isLoading;
}

void Loading::setLoadingIcon(const ResTIMG **textures, size_t texCount) {
    sLoadingIconAnimator.setTextures(textures, texCount);
}
void Loading::setLayout(J2DScreen *screen) { sLoadingScreen = screen; }
void Loading::setFrameRate(f32 fps) { sLoadingIconAnimator.setFrameRate(fps); }

#pragma region Implementation

static int baseIconX = 400;

void initLoadingScreen() {
    {
        JSUMemoryInputStream stream(sLoadingScreenBlo, sizeof(sLoadingScreenBlo));
        sLoadingScreen = new J2DScreen();
        sLoadingScreen->makeHiearachyPanes(sLoadingScreen, &stream, false, true, false,
                                               nullptr);
    }

    sLoadingIconTexture.mTexObj2.val[2] = 0;
    sLoadingIconTexture.storeTIMG(GetResourceTextureHeader(sLoadingIconTIMGs[0]));
    sLoadingIconTexture._50 = false;

    {
        J2DPicture *loadingIcon =
            reinterpret_cast<J2DPicture *>(sLoadingScreen->search('icon'));
        if (loadingIcon) {
            JUTRect rect = loadingIcon->mRect;
            loadingIcon->insert(&sLoadingIconTexture, 0, 1.0);
            loadingIcon->mRect = rect;
        }
    }

    sLoadingIconAnimator.setFrameRate(16.0f);
}

extern AspectRatioSetting gAspectRatioSetting;

void drawLoadingScreen(TApplication *app, const J2DOrthoGraph *ortho) {
    if (!sIsLoading || !sLoadingScreen)
        return;

    const int screenAdjustX = BetterSMS::getScreenRatioAdjustX();

    J2DScreen *screen = sLoadingScreen;

    auto loadingIcon = screen->search('icon');
    if (loadingIcon) {
        loadingIcon->mRect = {540 + screenAdjustX, 400, 572 + screenAdjustX, 432};
        sLoadingIconAnimator.process(reinterpret_cast<J2DPicture *>(loadingIcon));
    }

    screen->draw(0, 0, ortho);
}

#pragma endregion