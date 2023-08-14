#include <Dolphin/CARD.h>

#include "module.hxx"
#include "p_autosave.hxx"
#include "p_settings.hxx"

extern SavePromptsSetting gSavePromptSetting;

static bool gIsAutoSaveActive = false;

static u8 SMS_ALIGN(32) gSaveThreadStack[0x4000];
static OSThread gSaveThread;

static void *saveAllThread(void *arg) {
    if (SaveAllSettings() == CARD_ERROR_READY) {
        gIsAutoSaveActive = false;
    }
    return nullptr;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::triggerAutoSave() {
    if (gIsAutoSaveActive || gSavePromptSetting.getInt() != SavePromptsSetting::AUTO_SAVE)
        return false;

    s32 status = CARDCheck(gpCardManager->mChannel);
    if (status != CARD_ERROR_READY && status != CARD_ERROR_NOCARD)
        return false;

    gIsAutoSaveActive = true;
    OSCreateThread(&gSaveThread, saveAllThread, NULL, gSaveThreadStack + sizeof(gSaveThreadStack),
                   sizeof(gSaveThreadStack), 17, 0);
    OSResumeThread(&gSaveThread);
    return true;
}

// Implementation stuff

static J2DPicture sAutoSavePicture('auto', {0, 0, 0, 0});
static JUTTexture texture;
static J2DScreen *sAutoSaveScreen;
static SimpleTexAnimator sAutoSaveAnimator(sAutoSaveIconTIMGs, 20);

BETTER_SMS_FOR_CALLBACK void initAutoSaveIcon(TApplication *app) {
    auto *oldHeap = JKRHeap::sRootHeap->becomeCurrentHeap();

    /*sAutoSaveScreen = new J2DScreen(8, 'ROOT', {0, 0, 900, 480});
    OSReport("AutoSaveIcon: %p\n", sAutoSaveScreen);*/
    //{
    texture.mTexObj2.val[2] = 0;
    texture.storeTIMG(GetResourceTextureHeader(sAutoSaveIconTIMGs[0]));
    texture._50 = false;

    {
        const int screenAdjustX = BetterSMS::getScreenRatioAdjustX();

        sAutoSavePicture.insert(&texture, 0, 1.0f);
        sAutoSavePicture.mRect  = {0, 0, 32, 32};
        sAutoSavePicture.mAlpha = 255;

        /*sAutoSavePicture.mColorOverlay = {0, 0, 0, 255};
        sAutoSavePicture.mVertexColors[0] = {0, 0, 0, 100};
        sAutoSavePicture.mVertexColors[1] = {0, 0, 0, 100};
        sAutoSavePicture.mVertexColors[2] = {0, 0, 0, 255};
        sAutoSavePicture.mVertexColors[3] = {0, 0, 0, 255};*/
    }
    /*sAutoSaveScreen->mChildrenList.append(&sAutoSavePicture.mPtrLink);
}*/

    sAutoSaveAnimator.setFrameRate(20.0f);

    oldHeap->becomeCurrentHeap();
}

BETTER_SMS_FOR_CALLBACK void updateAutoSaveIcon(TApplication *app) {
    if (!gIsAutoSaveActive) {
        sAutoSaveAnimator.resetAnimation();
        return;
    }

    if (app->mContext != TApplication::CONTEXT_DIRECT_STAGE) {
        sAutoSaveAnimator.resetAnimation();
        return;
    }

    const int screenAdjustX = BetterSMS::getScreenRatioAdjustX();
    sAutoSavePicture.mRect  = {400 + screenAdjustX, 420, 432 + screenAdjustX, 452};

    if (sAutoSaveAnimator.getCurrentFrame() == 8 || sAutoSaveAnimator.getCurrentFrame() == 15 ||
        sAutoSaveAnimator.getCurrentFrame() == 19) {
        sAutoSaveAnimator.setFrameRate(20.0f / 3.0f);
    } else {
        sAutoSaveAnimator.setFrameRate(20.0f);
    }
}

BETTER_SMS_FOR_CALLBACK void drawAutoSaveIcon(TApplication *app, const J2DOrthoGraph *ortho) {
    if (!gIsAutoSaveActive)
        return;

    if (app->mContext != TApplication::CONTEXT_DIRECT_STAGE)
        return;

    const int screenAdjustX = BetterSMS::getScreenRatioAdjustX();

    ReInitializeGX();
    const_cast<J2DOrthoGraph *>(ortho)->setup2D();

    sAutoSaveAnimator.process(&sAutoSavePicture);
    sAutoSavePicture.draw(460 + screenAdjustX, 420, 32, 32, false, false, false);
}