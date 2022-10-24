#include <Dolphin/types.h>
#include <Dolphin/mem.h>
#include <Dolphin/string.h>
#include <Dolphin/CARD.h>
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
#include <SMS/option/CardManager.hxx>
#include <SMS/manager/FlagManager.hxx>

#include "libs/container.hxx"
#include "settings.hxx"
#include "module.hxx"
#include "icons.hxx"

#include "p_settings.hxx"

bool RumbleSetting::sRumbleFlag           = true;
int SoundSetting::sSoundValue             = SoundSetting::MONO;
bool SubtitleSetting::sSubtitleFlag       = true;
int AspectRatioSetting::sAspectRatioValue = AspectRatioSetting::FULL;
int FPSSetting::sFPSValue                 = FPSSetting::FPS_30; 

using namespace BetterSMS;

static TDictS<Settings::SettingsGroup *> sSettingsGroups;

// PRIVATE

void getSettingsGroups(TDictS<Settings::SettingsGroup *>::ItemList &out) {
    TDictS<Settings::SettingsGroup *>::ItemList temp;
    TDictS<Settings::SettingsGroup *>::ItemList tempCore;
    TDictS<Settings::SettingsGroup *>::ItemList tempGame;
    TDictS<Settings::SettingsGroup *>::ItemList tempMode;
    sSettingsGroups.items(temp);

    for (auto &item : temp) {
        if (strcmp(item.mKey, "Super Mario Sunshine")) {
            tempCore.insert(tempCore.begin(), item);
            continue;
        } else if (strcmp(item.mKey, "Better Sunshine Engine")) {
            tempCore.insert(tempCore.begin(), item);
            continue;
        }
        switch (item.mValue->getPriority()) {
        case Settings::Priority::CORE:
            tempCore.insert(tempCore.end(), item);
        case Settings::Priority::GAME:
            tempGame.insert(tempGame.end(), item);
        case Settings::Priority::MODE:
            tempMode.insert(tempMode.end(), item);
        }
    }

    for (auto &item : tempCore) {
        out.insert(out.end(), item);
    }

    for (auto &item : tempGame) {
        out.insert(out.end(), item);
    }

    for (auto &item : tempMode) {
        out.insert(out.end(), item);
    }
}

//

// PUBLIC

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
        mSettingScreen->mController->mState.mReadInput = true;
    }

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
    case State::GROUP:
    case State::SETTING: {
        if (mController->mButtons.mFrameInput & TMarioGamePad::B)
            mState = State::EXIT;
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
    mViewObjStageGroup             = new JDrama::TDStageGroup(display);
    mDisplay                       = display;
    mController                    = controller;
    mController->mState.mReadInput = false;
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
    return fader->mFadeStatus == TSMSFader::FADE_ON ? 5 : 1;
}

void SettingsDirector::initialize() {
    auto *stageObjGroup = reinterpret_cast<JDrama::TDStageGroup *>(mViewObjStageGroup);
    auto *rootObjGroup  = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Root View Objs");
    mViewObjRoot       = rootObjGroup;

    JDrama::TRect screenRect{0, 0, SMSGetTitleRenderWidth(), SMSGetTitleRenderHeight()};

    auto *group2D = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Group 2D");
    {
        mSettingScreen = new SettingsScreen(mController);
        group2D->mViewObjList.insert(group2D->mViewObjList.begin(), mSettingScreen);

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), group2D);
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

        groupGrad->mViewObjList.insert(groupGrad->mViewObjList.end(), mGradBG);

        /*auto *orthoProj = new JDrama::TOrthoProj();
        groupGrad->mViewObjList.insert(groupGrad->mViewObjList.end(), orthoProj);*/

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), groupGrad);
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

        screen->assignCamera(new JDrama::TOrthoProj());
        screen->assignViewObj(groupGrad);

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), screen);
        stageObjGroup->mViewObjList.insert(stageObjGroup->mViewObjList.end(), screen);
    }

    {
        auto *screen = new JDrama::TScreen(screenRect, "Screen 2D");

        screen->assignCamera(new JDrama::TOrthoProj());
        screen->assignViewObj(group2D);

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), screen);
        stageObjGroup->mViewObjList.insert(stageObjGroup->mViewObjList.end(), screen);
    }

    initializeLayout();
}

void SettingsDirector::initializeLayout() {
    const int screenOrthoWidth = BetterSMS::getScreenOrthoWidth();
    const int screenRenderWidth = BetterSMS::getScreenRenderWidth();
    const int screenRenderHeight  = 480;
    const int screenAdjustX = BetterSMS::getScreenRatioAdjustX();

    mSettingScreen->mScreen = new J2DScreen(8, 'ROOT', {0, 0, screenOrthoWidth, screenRenderHeight});
    {
        JUTTexture *mask      = new JUTTexture();
        mask->mTexObj2.val[2] = 0;
        mask->storeTIMG(GetResourceTextureHeader(gMaskBlack));
        mask->_50 = false;

        J2DPicture *maskTop = new J2DPicture('mskt', {0, 0, 0, 0});
        J2DPicture *maskBottom = new J2DPicture('mskb', {0, 0, 0, 0});

        maskTop->insert(mask, 0, 1.0f);
        maskBottom->insert(mask, 0, 1.0f);

        maskTop->mRect    = {-screenAdjustX, 0, screenOrthoWidth, 90};
        maskBottom->mRect = {-screenAdjustX, screenRenderHeight - 90, screenOrthoWidth, screenRenderHeight};

        maskTop->mAlpha = 160;
        maskBottom->mAlpha = 160;

        maskTop->mColorOverlay    = {0, 0, 0, 255};
        maskBottom->mColorOverlay = {0, 0, 0, 255};

        maskTop->mVertexColors[0] = {0, 0, 0, 100};
        maskTop->mVertexColors[1] = {0, 0, 0, 100};
        maskTop->mVertexColors[2] = {0, 0, 0, 255};
        maskTop->mVertexColors[3] = {0, 0, 0, 255};

        maskBottom->mVertexColors[0] = {0, 0, 0, 255};
        maskBottom->mVertexColors[1] = {0, 0, 0, 255};
        maskBottom->mVertexColors[2] = {0, 0, 0, 100};
        maskBottom->mVertexColors[3] = {0, 0, 0, 100};

        mSettingScreen->mScreen->mChildrenList.append(&maskTop->mPtrLink);
        mSettingScreen->mScreen->mChildrenList.append(&maskBottom->mPtrLink);

        J2DTextBox *label =
            new J2DTextBox('logo', {0, screenRenderHeight - 90, 600, screenRenderHeight}, gpSystemFont->mFont,
                           "Game Settings",
                           J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        label->mCharSizeX   = 24;
        label->mCharSizeY   = 24;
        label->mNewlineSize = 24;
        mSettingScreen->mScreen->mChildrenList.append(&label->mPtrLink);

        J2DTextBox *exitLabel = new J2DTextBox(
            'exit',
            {static_cast<int>(20 - getScreenRatioAdjustX()), screenRenderHeight - 90, static_cast<int>(100 - getScreenRatioAdjustX()), screenRenderHeight},
            gpSystemFont->mFont,
            "# Exit", J2DTextBoxHBinding::Left, J2DTextBoxVBinding::Center);
        mSettingScreen->mScreen->mChildrenList.append(&exitLabel->mPtrLink);
    }

    int i = 0;
    TDictS<Settings::SettingsGroup *>::ItemList settingsGroups;
    getSettingsGroups(settingsGroups);

    for (auto &group : settingsGroups) {
        auto *groupName = group.mValue->getName();

        J2DPane *groupPane =
            new J2DPane(19, ('p' << 24) | i, {0, 0, screenRenderWidth, screenRenderHeight});
        groupPane->mIsVisible = false;
        {

            char *groupTextBuf = new char[70];
            memset(groupTextBuf, 0, 70);

            if (settingsGroups.mSize == 1) {
                snprintf(groupTextBuf, 70, "    %s    ", groupName);
            } else {
                if (i == 0)
                    snprintf(groupTextBuf, 70, "    %s   >", groupName);
                else if (i == settingsGroups.mSize - 1)
                    snprintf(groupTextBuf, 70, "<   %s    ", groupName);
                else
                    snprintf(groupTextBuf, 70, "<   %s   >", groupName);
            }

            J2DTextBox *label =
                new J2DTextBox(('t' << 24) | i, {0, 0, 600, 90}, gpSystemFont->mFont, groupTextBuf,
                               J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
            label->mCharSizeX   = 24;
            label->mCharSizeY   = 24;
            label->mNewlineSize = 24;
            groupPane->mChildrenList.append(&label->mPtrLink);
        }

        auto *groupInfo          = new GroupInfo();
        groupInfo->mGroupPane    = groupPane;
        groupInfo->mSettingGroup = group.mValue;

        int n = 0;
        auto settingList = new JGadget::TList<SettingInfo *>();
        for (auto &setting : group.mValue->getSettings()) {
            J2DPane *settingPane =
                new J2DPane(19, ('q' << 24) | i, {0, 0, screenRenderWidth, screenRenderHeight});

            J2DTextBox *settingText =
                new J2DTextBox(('s' << 24) | n, {0, 0, 0, 0}, gpSystemFont->mFont, "",
                               J2DTextBoxHBinding::Left, J2DTextBoxVBinding::Center);

            J2DTextBox *settingTextBehind =
                new J2DTextBox(('b' << 24) | n, {0, 0, 0, 0}, gpSystemFont->mFont, "",
                               J2DTextBoxHBinding::Left, J2DTextBoxVBinding::Center);
            {
                char valueTextbuf[40];
                setting->getValueStr(valueTextbuf);

                char *settingTextBuf = new char[100];
                memset(settingTextBuf, 0, 100);
                snprintf(settingTextBuf, 100, "%s: %s", setting->getName(), valueTextbuf);

                const u8 alpha = setting->isUserEditable() ? 255 : 210;
                const u8 color = setting->isUserEditable() ? 255 : 140;

                settingText->mStrPtr                 = settingTextBuf;
                settingText->mCharSizeX              = 24;
                settingText->mCharSizeY              = 24;
                settingText->mNewlineSize            = 24;
                settingText->mGradientBottom         = {color, color, color, alpha};
                settingText->mGradientTop            = {color, color, color, alpha};

                centerTextBoxX(settingText, screenRenderWidth);
                settingText->mRect.mY1 = 110 + (28 * n);
                settingText->mRect.mY2 = 158 + (28 * n);

                settingTextBehind->mStrPtr         = settingTextBuf;
                settingTextBehind->mCharSizeX      = 24;
                settingTextBehind->mCharSizeY      = 24;
                settingTextBehind->mNewlineSize    = 24;
                settingTextBehind->mGradientBottom = {0, 0, 0, alpha};
                settingTextBehind->mGradientTop    = {0, 0, 0, alpha};

                centerTextBoxX(settingTextBehind, screenRenderWidth);
                settingTextBehind->mRect.mX1 += 2;
                settingTextBehind->mRect.mX2 += 2;
                settingTextBehind->mRect.mY1 = 110 + (28 * n) + 2;
                settingTextBehind->mRect.mY2 = 158 + (28 * n) + 2;

                settingPane->mChildrenList.append(&settingTextBehind->mPtrLink);
                settingPane->mChildrenList.append(&settingText->mPtrLink);
            }
            groupPane->mChildrenList.append(&settingPane->mPtrLink);

            auto *settingInfo            = new SettingInfo();
            settingInfo->mSettingTextBox = settingText;
            settingInfo->mSettingTextBoxBack = settingTextBehind;
            settingInfo->mSettingData    = setting;
            groupInfo->mSettingInfos.insert(groupInfo->mSettingInfos.end(), settingInfo);

            ++n;
        }

        /*{
            JUTTexture *texture      = new JUTTexture();
            texture->mTexObj2.val[2] = 0;
            texture->storeTIMG(GetResourceTextureHeader(gShineSpriteIconFrame1));
            texture->_50 = false;

            mSettingScreen->mShineIcon = new J2DPicture(('i' << 24) | n, {0, 0, 0, 0});
            mSettingScreen->mShineIcon->insert(texture, 0, 0.0f);
            mSettingScreen->mShineIcon->mRect.resize(32, 32);

            pane->mChildrenList.append(&mSettingScreen->mShineIcon->mPtrLink);
        }*/

        mSettingScreen->mScreen->mChildrenList.append(&groupPane->mPtrLink);
        mSettingScreen->mGroups.insert(mSettingScreen->mGroups.end(), groupInfo);

        if (i == 0) {
            mSettingScreen->mCurrentGroupInfo = groupInfo;
            mSettingScreen->mCurrentSettingInfo = mSettingScreen->getSettingInfo(0);
            mSettingScreen->mCurrentGroupInfo->mGroupPane->mIsVisible = true;
        }

        ++i;
    }
}

void SettingsDirector::saveSettings() {
    char statusBuf[40];
    char messageBuf[100];

    // TODO: Handle progress so we don't attempt saving every frame, and can have dialogs for errors...

    s32 status = CARDCheck(gpCardManager->mChannel);
    if (status == CARD_ERROR_READY) {
        // Save default flags (Rumble, Sound, Subtitles)
        {
            JSUMemoryOutputStream out(nullptr, 0);
            gpCardManager->getWriteStream(&out);
            TFlagManager::smInstance->save(out);
            gpCardManager->writeBlock(0);
        }


    }

    switch (status) {
    default:
    case CARD_ERROR_WRONGDEVICE:
        strncpy(messageBuf, "Unsupported Device", 40);
        CARDUnmount(gpCardManager->mChannel);
        break;
    case CARD_ERROR_NOCARD:
        strncpy(messageBuf, "Memory Card Not Found", 40);
        break;
    case CARD_ERROR_IOERROR:
        strncpy(messageBuf, "Memory Card Bad I/O", 40);
        CARDUnmount(gpCardManager->mChannel);
        break;
    case CARD_ERROR_BROKEN:
        strncpy(messageBuf, "Memory Card Corrupted", 40);
        CARDUnmount(gpCardManager->mChannel);
        break;
    case CARD_ERROR_FATAL_ERROR:
        strncpy(messageBuf, "Memory Card Fatal Error", 40);
        CARDUnmount(gpCardManager->mChannel);
        break;
    }



}

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