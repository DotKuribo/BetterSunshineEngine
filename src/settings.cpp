#include <Dolphin/types.h>
#include <Dolphin/mem.h>
#include <Dolphin/ctype.h>
#include <Dolphin/string.h>
#include <Dolphin/CARD.h>
#include <Dolphin/VI.h>

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

void initAllSettings(TApplication *app) {
    s32 cardChannel = CARD_SLOTA;
    s32 cardStatus  = MountCard(cardChannel);
    if (cardStatus < CARD_ERROR_READY) {
        cardChannel    = CARD_SLOTB;
        cardStatus = MountCard(cardChannel);
        if (cardStatus < CARD_ERROR_READY) {
            return;
        }
    }

    CARDFileInfo finfo;

    TDictS<Settings::SettingsGroup *>::ItemList groups;
    getSettingsGroups(groups);

    for (auto &group : groups) {
        if (group.mValue->getPriority() == Settings::Priority::CORE &&
            strcmp(group.mValue->getName(), "Super Mario Sunshine") == 0) {
            continue;
        }


        size_t saveDataStartData = 0;
        int ret                  = OpenSavedSettings(*group.mValue, cardChannel, finfo);
        if (ret < CARD_ERROR_READY) {
            CloseSavedSettings(*group.mValue, &finfo);
            return;
        }

        ReadSavedSettings(*group.mValue, &finfo);
        CloseSavedSettings(*group.mValue, &finfo);
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

/****************************************************************************
 * MountCard
 *
 * Mounts the memory card in the given slot.
 * CARD_Mount is called for a maximum of 10 tries
 * Returns the result of the last attempted CARD_Mount command.
 ***************************************************************************/

/*** Memory Card Work Area ***/
static SMS_ALIGN(32) u8 SysArea[CARD_WORKAREA];

s32 MountCard(s32 channel) {
    s32 ret   = -1;
    int tries = 0;
    int isMounted;

    s32 memsize, sectsize;

    // Mount the card, try several times as they are tricky
    while ((tries < 10) && (ret < 0)) {
        /*** We already initialized the Memory Card subsystem with CARD_Init() in
        select_memcard_slot(). Let's reset the EXI subsystem, just to make sure we have no problems
        mounting the card ***/
        CARDInit();

        /*** Mount the card ***/
        ret = CARDMount(channel, SysArea, detachCallback);
        if (ret >= 0)
            break;

        __CARDSync(channel);
        tries++;
    }

    /*** Make sure the card is really mounted ***/
    isMounted = CARDProbeEx(channel, &memsize, &sectsize);
    if (memsize > 0 && sectsize > 0)  // then we really mounted de card
    {
        return isMounted;
    }

    /*** If this point is reached, something went wrong ***/
    CARDUnmount(channel);
    return ret;
}

s32 OpenSavedSettings(Settings::SettingsGroup &group, const s32 channel, CARDFileInfo &infoOut) {
    auto &info = group.getSaveInfo();

    // Create and open save file for this group
    char normalizedPath[32];
    for (int i = 0; i < 32; ++i) {
        if (info.mSaveName[i] == ' ')
            normalizedPath[i] = '_';
        else
            normalizedPath[i] = tolower(info.mSaveName[i]);
    }

    if (info.mSaveGlobal)
        __CARDSetDiskID(&info.mGameCode);

    int ret;
    do {
        ret = CARDOpen(channel, normalizedPath, &infoOut);
    } while (ret == CARD_ERROR_BUSY);

    if (ret == CARD_ERROR_NOFILE) {
        int cret =
            CARDCreate(channel, normalizedPath, CARD_BLOCKS_TO_BYTES(info.mBlocks), &infoOut);
        if (cret < CARD_ERROR_READY) {
            if (info.mSaveGlobal)
                __CARDSetDiskID((void *)0x80000000);
            return cret;
        }
        UpdateSavedSettings(group, &infoOut);
    } else if (ret < CARD_ERROR_READY) {
        if (info.mSaveGlobal)
            __CARDSetDiskID((void *)0x80000000);
        return ret;
    }

    // We now have an open handle to the settings file
    return CARD_ERROR_READY;
}

s32 UpdateSavedSettings(Settings::SettingsGroup &group, CARDFileInfo *finfo) {
    auto &info                = group.getSaveInfo();

    const size_t saveDataSize = CARD_BLOCKS_TO_BYTES(info.mBlocks);
    char *saveBuffer          = new (32) char[saveDataSize];

    {
        CARDStat fstatus;

        // Work out status
        int statusRet = CARDGetStatus(finfo->mChannel, finfo->mFileNo, &fstatus);
        if (statusRet < CARD_ERROR_READY)
            return statusRet;

        fstatus.mGameCode = 'GMSB';
        fstatus.mCompany  = 0x3031;
        CARDSetBannerFmt(&fstatus, info.mBannerFmt);
        CARDSetIconAddr(&fstatus, CARD_DIRENTRY_SIZE);
        CARDSetCommentAddr(&fstatus, 4);
        for (s32 i = 0; i < info.mIconCount; ++i) {
            CARDSetIconFmt(&fstatus, i, info.mIconFmt);
            CARDSetIconSpeed(&fstatus, i, info.mIconSpeed);
        }
        fstatus.mLastModified = OSTicksToSeconds(OSGetTime());

        CARDSetStatus(finfo->mChannel, finfo->mFileNo, &fstatus);
    }

    // Reset data
    memset(saveBuffer + 4, 0, saveDataSize - 4);

    // Copy group name into save data
    snprintf(saveBuffer + 4, 32, "%s", info.mSaveName);

    // Copy date saved into save data
    {
        OSCalendarTime calendar;
        OSTicksToCalendarTime(OSGetTime(), &calendar);
        snprintf(saveBuffer + 36, 32, "Module Save Data (%lu/%lu/%lu)", calendar.mon + 1,
                 calendar.mday, calendar.year);
    }

    memcpy(saveBuffer + CARD_DIRENTRY_SIZE,
           reinterpret_cast<const u8 *>(info.mBannerImage) + info.mBannerImage->mTextureOffset,
           0xE00);
    memcpy(saveBuffer + CARD_DIRENTRY_SIZE + 0xE00,
           reinterpret_cast<const u8 *>(info.mIconTable) + info.mIconTable->mTextureOffset,
           0x500 * info.mIconCount);

    size_t dataPosOut = CARD_DIRENTRY_SIZE + 0xE00 + (0x500 * info.mIconCount);

    // Write contents to save file
    JSUMemoryOutputStream out(saveBuffer + dataPosOut, saveDataSize - dataPosOut);
    for (auto &setting : group.getSettings()) {
        switch (setting->getKind()) {
        case Settings::SingleSetting::ValueKind::INT:
        case Settings::SingleSetting::ValueKind::FLOAT:
            out.write(setting->getValue(), 4);
            break;
        case Settings::SingleSetting::ValueKind::BOOL:
            out.write(setting->getValue(), 1);
        }
    }

    for (size_t i = 0; i < saveDataSize; i += CARD_BLOCKS_TO_BYTES(1)) {
        s32 result = CARDWrite(finfo, saveBuffer, CARD_BLOCKS_TO_BYTES(1), i);
        if (result < CARD_ERROR_READY) {
            return result;
        }
    }

    return CARD_ERROR_READY;
}

s32 ReadSavedSettings(Settings::SettingsGroup &group, CARDFileInfo *finfo) {
    auto &info = group.getSaveInfo();

    const size_t saveDataSize = CARD_BLOCKS_TO_BYTES(info.mBlocks);
    char *saveBuffer          = new (32) char[saveDataSize];

    // Reset data
    memset(saveBuffer + 4, 0, saveDataSize - 4);

    for (size_t i = 0; i < saveDataSize; i += CARD_BLOCKS_TO_BYTES(1)) {
        s32 result = CARDRead(finfo, saveBuffer, CARD_BLOCKS_TO_BYTES(1), i);
        if (result < CARD_ERROR_READY) {
            return result;
        }
    }

    size_t dataPosOut = CARD_DIRENTRY_SIZE + 0xE00 + (0x500 * info.mIconCount);

    // Write contents to save file
    JSUMemoryInputStream in(saveBuffer + dataPosOut, saveDataSize - dataPosOut);
    for (auto &setting : group.getSettings()) {
        switch (setting->getKind()) {
        case Settings::SingleSetting::ValueKind::INT:
            int x;
            in.read(&x, 4);
            setting->setInt(x);
            break;
        case Settings::SingleSetting::ValueKind::FLOAT:
            float f;
            in.read(&f, 4);
            setting->setFloat(f);
            break;
        case Settings::SingleSetting::ValueKind::BOOL:
            bool b;
            in.read(&b, 1);
            setting->setBool(b);
            break;
        }
    }

    return CARD_ERROR_READY;
}

s32 CloseSavedSettings(const Settings::SettingsGroup &group, CARDFileInfo *finfo) {
    auto &info = group.getSaveInfo();
    if (info.mSaveGlobal)
        __CARDSetDiskID(&info.mGameCode);
    s32 ret = CARDClose(finfo);
    if (info.mSaveGlobal)
        __CARDSetDiskID((void *)0x80000000);
    return ret;
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

        mState = State::CONTROL;
        return 0;
    }

    TDirector::direct();

    switch (mState) {
    case State::INIT:
        break;
    case State::CONTROL:
        mSettingScreen->mPerformFlags &= ~0b0001;  // Enable input by default;
        mSaveErrorPanel->mPerformFlags |= 0b1011;  // Disable view and input by default
        if ((mController->mButtons.mFrameInput & TMarioGamePad::B)) {
            mState = State::SAVE_START;
            mSaveErrorPanel->appear();
        }
        break;
    case State::SAVE_START:
        saveSettings();
        break;
    case State::SAVE_BUSY:
        mSettingScreen->mPerformFlags |= 0b0001;  // Disable input
        mSaveErrorPanel->mPerformFlags &= ~0b1011;  // Enable view and input
        break;
    case State::SAVE_FAIL:
        [[fallthrough]];
    case State::SAVE_SUCCESS:
        mSaveErrorPanel->disappear();
        mState = State::EXIT;
        [[fallthrough]];	
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

extern RumbleSetting gRumbleSetting;
extern SoundSetting gSoundSetting;
extern SubtitleSetting gSubtitleSetting;

void SettingsDirector::initialize() {

    gRumbleSetting.setBool(TFlagManager::smInstance->getBool(0x90000));
    gSoundSetting.setInt(TFlagManager::smInstance->getFlag(0xA0000));
    gSubtitleSetting.setBool(TFlagManager::smInstance->getBool(0x90001));

    initializeDramaHierarchy();
    initializeSettingsLayout();
    initializeErrorLayout();
}

void SettingsDirector::initializeDramaHierarchy() {
    auto *stageObjGroup = reinterpret_cast<JDrama::TDStageGroup *>(mViewObjStageGroup);
    auto *rootObjGroup  = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Root View Objs");
    mViewObjRoot        = rootObjGroup;

    JDrama::TRect screenRect{0, 0, SMSGetTitleRenderWidth(), SMSGetTitleRenderHeight()};

    auto *group2D = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Group 2D");
    {
        mSettingScreen = new SettingsScreen(mController);
        group2D->mViewObjList.insert(group2D->mViewObjList.end(), mSettingScreen);

        mSaveErrorPanel = new SaveErrorPanel(this, mController);
        group2D->mViewObjList.insert(group2D->mViewObjList.end(), mSaveErrorPanel);

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
}

void SettingsDirector::initializeSettingsLayout() {
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

static char sErrorTag[64];

void SettingsDirector::initializeErrorLayout() {
    const int screenOrthoWidth   = BetterSMS::getScreenOrthoWidth();
    const int screenRenderWidth  = BetterSMS::getScreenRenderWidth();
    const int screenRenderHeight = 480;
    const int screenAdjustX      = BetterSMS::getScreenRatioAdjustX();

    mSaveErrorPanel->mScreen =
        new J2DScreen(8, 'ROOT', {0, 0, screenOrthoWidth, screenRenderHeight});
    {
        JUTTexture *mask      = new JUTTexture();
        mask->mTexObj2.val[2] = 0;
        mask->storeTIMG(GetResourceTextureHeader(gMaskBlack));
        mask->_50 = false;

        J2DPane *rootPane = new J2DPane(19, 'root', {0, 0, 400, 280});
        mSaveErrorPanel->mScreen->mChildrenList.append(&rootPane->mPtrLink);

        mSaveErrorPanel->mAnimatedPane        = new TBoundPane(rootPane, {0, 0, 400, 280});
        mSaveErrorPanel->mAnimatedPane->mPane = rootPane;

        J2DPicture *maskPanel    = new J2DPicture('mask', {0, 0, 0, 0});
        {
            maskPanel->insert(mask, 0, 1.0f);
            maskPanel->mRect            = {0, 0, 400, 300};
            maskPanel->mAlpha           = 210;
            maskPanel->mColorOverlay    = {0, 0, 0, 255};
            maskPanel->mVertexColors[0] = {20, 0, 0, 255};
            maskPanel->mVertexColors[1] = {20, 0, 0, 255};
            maskPanel->mVertexColors[2] = {20, 0, 0, 255};
            maskPanel->mVertexColors[3] = {20, 0, 0, 255};
        }
        rootPane->mChildrenList.append(&maskPanel->mPtrLink);

        mSaveErrorPanel->mErrorHandlerPane             = new J2DPane(19, 'err_', {0, 0, 400, 280});
        mSaveErrorPanel->mErrorHandlerPane->mIsVisible = false;
        {
            mSaveErrorPanel->mErrorTextBox =
                new J2DTextBox('errl', {12, 16, 388, 40}, gpSystemFont->mFont, "UNKNOWN ERROR",
                                        J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Top);
            {
                mSaveErrorPanel->mErrorTextBox->mStrPtr         = sErrorTag;
                mSaveErrorPanel->mErrorTextBox->mCharSizeX      = 21;
                mSaveErrorPanel->mErrorTextBox->mCharSizeY      = 24;
                mSaveErrorPanel->mErrorTextBox->mGradientBottom = {160, 190, 20, 255};
                mSaveErrorPanel->mErrorTextBox->mGradientBottom = {160, 190, 20, 255};
            }
            mSaveErrorPanel->mErrorHandlerPane->mChildrenList.append(
                &mSaveErrorPanel->mErrorTextBox->mPtrLink);

            J2DTextBox *description = new J2DTextBox(
                'desc', {20, 50, 380, 230}, gpSystemFont->mFont,
                "Something went wrong when saving the settings.\nWould you like to try again?",
                J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
            {
                description->mCharSizeX = 21;
                description->mCharSizeY = 24;
            }
            mSaveErrorPanel->mErrorHandlerPane->mChildrenList.append(&description->mPtrLink);

            mSaveErrorPanel->mChoiceBoxes[0] =
                new J2DTextBox('exit', {80, 240, 202, 268}, gpSystemFont->mFont, "Exit",
                               J2DTextBoxHBinding::Left, J2DTextBoxVBinding::Bottom);
            {
                mSaveErrorPanel->mChoiceBoxes[0]->mCharSizeX = 24;
                mSaveErrorPanel->mChoiceBoxes[0]->mCharSizeY = 24;
            }
            mSaveErrorPanel->mErrorHandlerPane->mChildrenList.append(
                &mSaveErrorPanel->mChoiceBoxes[0]->mPtrLink);

            mSaveErrorPanel->mChoiceBoxes[1] =
                new J2DTextBox('save', {250, 240, 392, 268}, gpSystemFont->mFont, "Retry",
                               J2DTextBoxHBinding::Left, J2DTextBoxVBinding::Bottom);
            {
                mSaveErrorPanel->mChoiceBoxes[1]->mCharSizeX = 24;
                mSaveErrorPanel->mChoiceBoxes[1]->mCharSizeY = 24;
            }
            mSaveErrorPanel->mErrorHandlerPane->mChildrenList.append(
                &mSaveErrorPanel->mChoiceBoxes[1]->mPtrLink);
        }
        rootPane->mChildrenList.append(&mSaveErrorPanel->mErrorHandlerPane->mPtrLink);

        mSaveErrorPanel->mSaveTryingPane             = new J2DPane(19, 'save', {0, 0, 400, 280});
        mSaveErrorPanel->mSaveTryingPane->mIsVisible = true;
        {
            J2DTextBox *description = new J2DTextBox(
                'desc', {20, 50, 380, 230}, gpSystemFont->mFont,
                "Saving to the memory card...",
                J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
            {
                description->mCharSizeX = 21;
                description->mCharSizeY = 24;
            }
            mSaveErrorPanel->mSaveTryingPane->mChildrenList.append(&description->mPtrLink);
        }
        rootPane->mChildrenList.append(&mSaveErrorPanel->mSaveTryingPane->mPtrLink);
    }
}

void *SettingsDirector::saveThreadFunc(void *data) {
    auto director = reinterpret_cast<SettingsDirector *>(data);
    director->saveSettings_();
    return nullptr;
}

static char sSaveThreadStack[0x4000];

void SettingsDirector::saveSettings() {
    OSCreateThread(&gSetupThread, saveThreadFunc, this, gpSetupThreadStack + 0x10000, 0x10000, 17,
                   0);
    OSResumeThread(&gSetupThread);
    mState = State::SAVE_BUSY;
}

void SettingsDirector::saveSettings_() {
    char statusBuf[40];
    char messageBuf[100];

    // TODO: Handle progress so we don't attempt saving every frame, and can have dialogs for
    // errors...

    s32 cardChannel = gpCardManager->mChannel;
    s32 cardStatus  = MountCard(cardChannel);
    if (cardStatus < CARD_ERROR_READY) {
        failSave(cardStatus);
        return;
    }

    // Save default flags (Rumble, Sound, Subtitles)
    {
        JSUMemoryOutputStream out(nullptr, 0);
        gpCardManager->getOptionWriteStream(&out);
        TFlagManager::smInstance->saveOption(out);
        gpCardManager->writeOptionBlock();
    }

    CARDFileInfo finfo;

    JGadget::TList<TDictS<BetterSMS::Settings::SettingsGroup *>::Item> groups;
    getSettingsGroups(groups);

    for (auto &group : groups) {
        if (group.mValue->getPriority() == Settings::Priority::CORE &&
            strcmp(group.mValue->getName(), "Super Mario Sunshine") == 0) {
            continue;
        }

        size_t saveDataStartData = 0;
        int ret = OpenSavedSettings(*group.mValue, cardChannel, finfo);
        if (ret < CARD_ERROR_READY) {
            CloseSavedSettings(*group.mValue, &finfo);
            failSave(ret);
            return;
        }

        UpdateSavedSettings(*group.mValue, &finfo);
        CloseSavedSettings(*group.mValue, &finfo);
    }

    mState = State::SAVE_SUCCESS;
    mErrorCode = CARD_ERROR_READY;
    return;
}

void SettingsDirector::failSave(int errorcode) {
    CARDUnmount(gpCardManager->mChannel);
    mSaveErrorPanel->switchScreen();
    mErrorCode = errorcode;
    strncpy(sErrorTag, getErrorString(errorcode), 64);
    return;
}

const char *SettingsDirector::getErrorString(int errorcode) {
    switch (errorcode) {
    default:
    case CARD_ERROR_WRONGDEVICE:
        return "Unsupported Device";
    case CARD_ERROR_NOCARD:
        return "Memory Card Not Found";
    case CARD_ERROR_IOERROR:
        return "Memory Card Bad I/O";
    case CARD_ERROR_BROKEN:
        return "Memory Card Corrupted";
    case CARD_ERROR_FATAL_ERROR:
        return "Memory Card Fatal Error";
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