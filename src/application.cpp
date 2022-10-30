#include <Dolphin/DVD.h>
#include <Dolphin/types.h>


#include <SMS/GC2D/SelectDir.hxx>
#include <SMS/Player/MarioGamePad.hxx>
#include <SMS/System/GCLogoDir.hxx>
#include <SMS/System/MenuDirector.hxx>
#include <SMS/System/MovieDirector.hxx>
#include <SMS/System/RenderModeObj.hxx>
#include <SMS/System/Resolution.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/manager/FlagManager.hxx>
#include <SMS/manager/RumbleManager.hxx>
#include <SMS/option/CardManager.hxx>


#include "application.hxx"
#include "libs/container.hxx"
#include "module.hxx"
#include "p_settings.hxx"


using namespace BetterSMS;

static TDictI<Application::ContextCallback> sContextCBs;

SMS_NO_INLINE bool BetterSMS::Application::isContextRegistered(u8 context) {
    return sContextCBs.hasKey(context);
}

SMS_NO_INLINE bool BetterSMS::Application::registerContextCallback(u8 context, ContextCallback cb) {
    if (sContextCBs.hasKey(context))
        return false;
    sContextCBs.set(context, cb);
    return true;
}

SMS_NO_INLINE bool BetterSMS::Application::deregisterContextCallback(u8 context) {
    if (sContextCBs.hasKey(context))
        return false;
    sContextCBs.pop(context);
    return true;
}

bool BetterAppContextGameBoot(TApplication *app) {
    SMSSetupGCLogoRenderingInfo(app->mDisplay);
    return false;
}

bool BetterAppContextGameBootLogo(TApplication *app) {
    SMSSetupGCLogoRenderingInfo(app->mDisplay);

    auto *director = new TGCLogoDir();
    app->mDirector = director;

    director->setup(app->mDisplay, app->mGamePad1);
    return false;
}

bool BetterAppContextDirectMovie(TApplication *app) {
    SMSSetupMovieRenderingInfo(app->mDisplay);

    app->mFader->setDisplaySize(SMSGetGameRenderWidth(), SMSGetGameRenderHeight());

    auto *director = new TMovieDirector();
    app->mDirector = director;

    director->setup(app->mDisplay, app->mGamePad1);
    return false;
}

bool BetterAppContextGameBootIntro(TApplication *app) {
    app->mCutSceneID           = 9;
    app->mNextScene.mAreaID    = 15;
    app->mNextScene.mEpisodeID = 0;
    app->mNextScene.mFlag.mVal = 0;
    BetterAppContextDirectMovie(app);
    return false;
}

bool BetterAppContextDirectStage(TApplication *app) {
    if (!app->checkAdditionalMovie()) {
        SMSSetupGameRenderingInfo(app->mDisplay, (app->_44 & 1) != 0);

        app->mFader->setDisplaySize(SMSGetGameRenderWidth(), SMSGetGameRenderHeight());

        auto *director = new TMarDirector();
        app->mDirector = director;

        bool skipLoop = director->setup(app->mDisplay, &app->mGamePad1, app->mCurrentScene.mAreaID,
                                        app->mCurrentScene.mEpisodeID) != 0;

        if (skipLoop)
            app->mContext = 4;
        return skipLoop;
    } else {
        BetterAppContextDirectMovie(app);
        return false;
    }
}

bool BetterAppContextDirectShineSelect(TApplication *app) {
    SMSSetupTitleRenderingInfo(app->mDisplay);

    app->mFader->setDisplaySize(SMSGetTitleRenderWidth(), SMSGetTitleRenderHeight());

    auto *director = new TSelectDir();
    app->mDirector = director;

    director->setup(app->mDisplay, app->mGamePad1, app->mCurrentScene.mAreaID);
    return false;
}

bool BetterAppContextDirectLevelSelect(TApplication *app) {
    SMSSetupTitleRenderingInfo(app->mDisplay);

    app->mFader->setDisplaySize(SMSGetTitleRenderWidth(), SMSGetTitleRenderHeight());

    auto *director = new TMenuDirector();
    app->mDirector = director;

    director->setup(app->mDisplay, app->mGamePad1);

    TFlagManager::smInstance->setFlag(0x20001, 3);

    app->mCurrentScene.set(1, 0, 0);
    return false;
}

bool BetterAppContextDirectSettingsMenu(TApplication *app) {
    SMSSetupTitleRenderingInfo(app->mDisplay);

    app->mFader->setDisplaySize(SMSGetTitleRenderWidth(), SMSGetTitleRenderHeight());

    auto *director = new SettingsDirector();
    app->mDirector = director;

    director->setup(app->mDisplay, app->mGamePad1);

    app->mCurrentScene.set(15, 0, 0);
    app->mNextScene.set(15, 0, 0);
    return false;
}

#define SMS_CHECK_RESET_FLAG(gamepad) (((1 << (gamepad)->mPort) & TMarioGamePad::mResetFlag) == 1)

extern void gameBootCallbackHandler(TApplication *app);
extern JKRHeap *exitStageCallbacks();

void BetterApplicationProcess(TApplication *app) {
    bool exitLoop   = false;
    u8 delayContext = 1;
    do {
        auto *cb = sContextCBs.get(app->mContext);
        SMS_ASSERT(cb, "Application attempted to fetch a context handler but it wasn't found!");

        exitLoop = (*cb)(app);
        if (!exitLoop)
            delayContext = app->gameLoop();

        if (app->mDirector)
            delete app->mDirector;
        app->mDirector = nullptr;

        if (app->mContext == TApplication::CONTEXT_GAME_BOOT_LOGO) {
            if (!SMS_CHECK_RESET_FLAG(app->mGamePad1)) {
                app->initialize_nlogoAfter();
                #if 1
                if (BetterSMS::isDebugMode())
                    delayContext = TApplication::CONTEXT_DIRECT_LEVEL_SELECT;
                #endif
            }
        } else if (app->mContext == TApplication::CONTEXT_GAME_BOOT) {
            if (!SMS_CHECK_RESET_FLAG(app->mGamePad1)) {
                app->initialize_bootAfter();
                gameBootCallbackHandler(app);
            }
        } else {
            exitStageCallbacks();
            app->mCurrentHeap->freeAll();
        }

        SMSRumbleMgr->reset();

        if (SMS_CHECK_RESET_FLAG(app->mGamePad1)) {
            TMarioGamePad::mResetFlag = 0;
            JUTGamePad::recalibrate(0xF0000000);

            exitLoop = DVDCheckDisk();
            if (!exitLoop) {
                delayContext = TApplication::CONTEXT_GAME_SHUTDOWN;
                app->_44 |= 2;
            } else {
                if (app->mContext == TApplication::CONTEXT_GAME_BOOT ||
                    app->mContext == TApplication::CONTEXT_GAME_BOOT_LOGO) {
                    app->mContext = TApplication::CONTEXT_GAME_SHUTDOWN;
                } else if (app->mContext != TApplication::CONTEXT_GAME_SHUTDOWN) {
                    #if 1
                    delayContext = BetterSMS::isDebugMode()
                                        ? TApplication::CONTEXT_DIRECT_LEVEL_SELECT
                                        : TApplication::CONTEXT_DIRECT_MOVIE;
                    #else
                    delayContext = TApplication::CONTEXT_GAME_INTRO;
                    #endif
                    gpCardManager->unmount();
                }
            }
        }
        app->mContext      = delayContext;
        app->mPrevScene    = app->mCurrentScene;
        app->mCurrentScene = app->mNextScene;
    } while (app->mContext != TApplication::CONTEXT_GAME_SHUTDOWN);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80005624, 0, 0, 0), BetterApplicationProcess);