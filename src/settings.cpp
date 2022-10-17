#include <Dolphin/types.h>
#include <JSystem/JDrama/JDRViewObjPtrListT.hxx>
#include <JSystem/JDrama/JDRCamera.hxx>
#include <JSystem/JDrama/JDRDStage.hxx>
#include <JSystem/JDrama/JDRDStageGroup.hxx>
#include <JSystem/JDrama/JDRScreen.hxx>
#include <JSystem/JUtility/JUTColor.hxx>
#include <JSystem/JUtility/JUTRect.hxx>
#include <JSystem/J2D/J2DPicture.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <JSystem/JUtility/JUTTexture.hxx>
#include <SMS/manager/RumbleManager.hxx>
#include <SMS/screen/SMSFader.hxx>
#include <SMS/sound/MSBGM.hxx>
#include <SMS/sound/MSound.hxx>
#include <SMS/sound/MSoundSESystem.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/Camera/CubeManagerBase.hxx>
#include <SMS/MarioUtil/DrawUtil.hxx>
#include <SMS/MarioUtil/gd-reinit-gx.hxx>
#include <SMS/System/Resolution.hxx>

#include "libs/container.hxx"
#include "settings.hxx"
#include "globals.hxx"
#include "module.hxx"
#include "icons.hxx"

#include "p_settings.hxx"

int SoundSetting::sSoundValue = 0;

using namespace BetterSMS;

static TDictS<Settings::SettingsGroup *> sSettingsGroups;

Settings::SettingsGroup *BetterSMS::Settings::getGroup(const char *name) {
    return sSettingsGroups.get(name);
}

bool BetterSMS::Settings::isGroupRegistered(const char *name) {
    return sSettingsGroups.hasKey(name);
}

bool BetterSMS::Settings::registerGroup(const char *name, Settings::SettingsGroup *group) {
    if (sSettingsGroups.hasKey(name))
        return false;
    sSettingsGroups.set(name, group);
    return true;
}

bool BetterSMS::Settings::deregisterGroup(const char *name) {
    if (!sSettingsGroups.hasKey(name))
        return false;
    sSettingsGroups.pop(name);
    return true;
}

SettingsDirector::~SettingsDirector() { gpMSound->exitStage(); }

s32 SettingsDirector::direct() {
    s32 ret = 1;

    int *joinBuf[2];

    //mController->read();
    //mController->updateMeaning();
    TSMSFader *fader = gpApplication.mFader;
    if (fader->mFadeStatus == TSMSFader::FADE_OFF) {
        processInput();
    }
    processAnimations();

    if (mState == State::INIT) {
        if (!OSIsThreadTerminated(&gSetupThread))
            return 0;
        OSJoinThread(&gSetupThread, (void **)joinBuf);
        /*if (joinBuf[0] == nullptr)
            return 5;*/
        fader->startFadeinT(0.3f);

        gpMSound->initSound();
        gpMSound->enterStage(MS_WAVE_DELFINO_PLAZA, 1, 2);
        MSBgm::startBGM(BGM_UNDERGROUND);

        mState = State::GROUP;
        return 0;
    }

    TDirector::direct();

    switch (mState) {
    case State::INIT:
        break;
    case State::GROUP: {
        break;
    }
    case State::SETTING: {
        break;
    }
    case State::EXIT: {
        ret = exit();
        break;
    }
    }
    return ret;
}

void SettingsDirector::setup(JDrama::TDisplay *display, TMarioGamePad *controller) {
    mViewObjStageGroup = new JDrama::TDStageGroup(display);
    mDisplay = display;
    mController = controller;
    SMSRumbleMgr->reset();
    OSCreateThread(&gSetupThread, setupThreadFunc, this, gpSetupThreadStack + 0x10000,
                   0x10000, 17, 0);
    OSResumeThread(&gSetupThread);
}

void *SettingsDirector::setupThreadFunc(void *param) {
    auto *director = reinterpret_cast<SettingsDirector *>(param);
    director->initialize();
    return nullptr;
}

s32 SettingsDirector::exit() {
    TSMSFader *fader = gpApplication.mFader;
    if (fader->mFadeStatus == TSMSFader::FADE_OFF)
        gpApplication.mFader->startFadeoutT(0.3f);
    return fader->mFadeStatus == TSMSFader::FADE_ON;
}

void SettingsDirector::initialize() {
    mGroupID   = 0;
    mSettingID = 0;

    auto *stageObjGroup = reinterpret_cast<JDrama::TDStageGroup *>(mViewObjStageGroup);
    auto *rootObjGroup  = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Root View Objs");
    mViewObjRoot       = rootObjGroup;

    JDrama::TRect screenRect{0, 0, SMSGetTitleRenderWidth(), SMSGetTitleRenderHeight()};

    {
        auto *group2D = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Group 2D");

        mScreen = new SettingsScreen();

        group2D->mViewObjList.insert(group2D->mViewObjList.begin(), mScreen);

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), group2D);
        stageObjGroup->mViewObjList.insert(stageObjGroup->mViewObjList.end(), group2D);
    }

    {
        auto *group3D = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Group 3D");

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), group3D);
        stageObjGroup->mViewObjList.insert(stageObjGroup->mViewObjList.end(), group3D);
    }

    {
        auto *group2DParticle = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Group 2D Particle");

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), group2DParticle);
        stageObjGroup->mViewObjList.insert(stageObjGroup->mViewObjList.end(), group2DParticle);
    }

    auto *groupGrad = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Group Grad");
    {
        mGradBG = new TSelectGrad("<TSelectGrad>");
        mGradBG->setStageColor(1);

        groupGrad->mViewObjList.insert(groupGrad->mViewObjList.begin(), mGradBG);

        auto *orthoProj = new JDrama::TOrthoProj();
        groupGrad->mViewObjList.insert(groupGrad->mViewObjList.begin(), orthoProj);

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), groupGrad);
        stageObjGroup->mViewObjList.insert(stageObjGroup->mViewObjList.end(), groupGrad);
    }

    {
        auto *stageDisp = new JDrama::TDStageDisp("<DStageDisp>", {0});

        auto *efbCtrl = stageDisp->getEfbCtrlDisp();
        efbCtrl->setSrcRect(screenRect);

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), stageDisp);
        stageObjGroup->mViewObjList.insert(stageObjGroup->mViewObjList.end(), stageDisp);
    }

    {
        auto *screen = new JDrama::TScreen(screenRect, "Screen Grad");

        auto *orthoProj = new JDrama::TOrthoProj();
        screen->assignCamera(orthoProj);
        screen->assignViewObj(groupGrad);

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), screen);
        stageObjGroup->mViewObjList.insert(stageObjGroup->mViewObjList.end(), screen);
    }

    initializeLayout();
}

void SettingsDirector::initializeLayout() {
    mGroupID   = 0;
    mSettingID = 0;

    mScreen->mScreen.mRect.set(0, 0, 640, 448);

    {
        JUTTexture *mask      = new JUTTexture();
        mask->mTexObj2.val[2] = 0;
        mask->storeTIMG(GetResourceTextureHeader(gMaskBlack));
        mask->_50 = false;

        J2DPicture *maskTop    = new J2DPicture('mskt', {0, 0, BetterSMS::getScreenWidth(), 90});
        J2DPicture *maskBottom = new J2DPicture('mskb', {0, SMSGetTitleVideoHeight() - 90, BetterSMS::getScreenWidth(),
                                    SMSGetTitleVideoHeight()});

        JUTTexture *mask2      = new JUTTexture();
        mask2->mTexObj2.val[2] = 0;
        mask2->storeTIMG(GetResourceTextureHeader(gMaskBlack));
        mask2->_50 = false;

        maskTop->insert(mask, 0, 1.0f);
        maskBottom->insert(mask2, 0, 1.0f);

        mScreen->mScreen.mChildrenList.append(&maskTop->mPtrLink);
        mScreen->mScreen.mChildrenList.append(&maskBottom->mPtrLink);
    }

    int i = 0;
    TDictS<Settings::SettingsGroup *>::ItemList settingsGroups;
    sSettingsGroups.items(settingsGroups);

    for (auto &group : settingsGroups) {
        auto *groupName = group.mValue->getName();

        J2DPane *pane = new J2DPane(19, ('p' << 24) | i, {320 * i, 0, 640, 480});
        {
            J2DTextBox *label = new J2DTextBox(('t' << 24) | i, {0, 40, 640, 32}, gpSystemFont->mFont, groupName,
                               J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
            pane->mChildrenList.append(&label->mPtrLink);
        }

        size_t n = 0;
        for (auto &setting : group.mValue->getSettings()) {
            {
                JUTTexture *texture      = new JUTTexture();
                texture->mTexObj2.val[2] = 0;
                texture->storeTIMG(GetResourceTextureHeader(gShineSpriteIconFrame1));
                texture->_50 = false;

                J2DPicture *settingSprite = new J2DPicture(('i' << 24) | n, {40, 120, 16, 16});
                settingSprite->insert(texture, 0, 1.0f);

                pane->mChildrenList.append(&settingSprite->mPtrLink);
            }

            {
                J2DTextBox *settingText = new J2DTextBox(
                    ('s' << 24) | n, {80, 120, 560, 16}, gpSystemFont->mFont, groupName,
                                   J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
                settingText->mCharSizeX = 18;
                settingText->mCharSizeY = 20;
                settingText->mNewlineSize = 20;
                pane->mChildrenList.append(&settingText->mPtrLink);
            }
            ++n;
        }

        mScreen->mScreen.mChildrenList.append(&pane->mPtrLink);
        mScreen->mGroupPanes.insert(mScreen->mGroupPanes.end(), pane);
        ++i;
    }
}

void SettingsDirector::processInput() {}

void SettingsDirector::processAnimations() {}

static s32 checkForSettingsMenu(TMarDirector *director) {
    s32 ret = director->changeState();
    if (director->mAreaID == 15 && director->mEpisodeID == 0) {
        if (gpCubeCamera->getInCubeNo(*(Vec *)gpMarioPos) > 0) {
            TSMSFader *fader = gpApplication.mFader;
            if (fader->mFadeStatus == TSMSFader::FADE_OFF) {
                fader->startFadeoutT(0.4f);
                //gpMSound->fadeOutAllSound(0.4f * SMSGetVSyncTimesPerSec());
            }
            if (fader->mFadeStatus == TSMSFader::FADE_ON) {
                ret = 10;
                //gpMSound->exitStage();
            }
        }
    }
    return ret;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80299D0C, 0, 0, 0), checkForSettingsMenu);