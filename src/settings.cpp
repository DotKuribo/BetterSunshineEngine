#include <Dolphin/CARD.h>
#include <Dolphin/VI.h>
#include <Dolphin/ctype.h>
#include <Dolphin/mem.h>
#include <Dolphin/string.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DPicture.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <JSystem/JDrama/JDRCamera.hxx>
#include <JSystem/JDrama/JDRDStage.hxx>
#include <JSystem/JDrama/JDRDStageGroup.hxx>
#include <JSystem/JDrama/JDRScreen.hxx>
#include <JSystem/JDrama/JDRViewObjPtrListT.hxx>
#include <JSystem/JUtility/JUTColor.hxx>
#include <JSystem/JUtility/JUTRect.hxx>
#include <JSystem/JUtility/JUTTexture.hxx>

#include <SMS/Camera/CubeManagerBase.hxx>
#include <SMS/GC2D/SMSFader.hxx>
#include <SMS/MSound/MSBGM.hxx>
#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Manager/FlagManager.hxx>
#include <SMS/Manager/RumbleManager.hxx>
#include <SMS/MarioUtil/DrawUtil.hxx>
#include <SMS/MarioUtil/gd-reinit-gx.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/System/CardManager.hxx>
#include <SMS/System/Resolution.hxx>

#include "libs/constmath.hxx"
#include "libs/container.hxx"
#include "libs/global_vector.hxx"
#include "libs/string.hxx"
#include "module.hxx"
#include "settings.hxx"

#include "p_icons.hxx"
#include "p_module.hxx"
#include "p_settings.hxx"
#include <libs/scoped_ptr.hxx>

/*** Memory Card Work Area ***/
static SMS_ALIGN(32) u8 SysArea[CARD_WORKAREA];

static bool sIsMounted = false;
static s32 sChannel    = 0;

static void detachCallback_(s32 channel, s32 res) { sIsMounted = false; }

BETTER_SMS_FOR_EXPORT const char *Settings::getGroupName(const Settings::SettingsGroup &group) {
    if (!group.mModule)
        return "Super Mario Sunshine";
    return group.mModule->mName;
}

BETTER_SMS_FOR_EXPORT s32 Settings::mountCard() {
    sIsMounted = true;

    s32 check;

    check = CARDCheck(CARD_SLOTA);
    if (check >= CARD_ERROR_BUSY) {
        sChannel = CARD_SLOTA;
        return check;
    }

    check = CARDCheck(CARD_SLOTB);
    if (check >= CARD_ERROR_BUSY) {
        sChannel = CARD_SLOTB;
        return check;
    }

    check = CARDMount(CARD_SLOTA, SysArea, detachCallback_);
    if (check == CARD_ERROR_READY) {
        sChannel = CARD_SLOTA;
        return check;
    }

    check = CARDMount(CARD_SLOTB, SysArea, detachCallback_);
    if (check == CARD_ERROR_READY) {
        sChannel = CARD_SLOTB;
        return check;
    }

    sIsMounted = false;
    return check;
}

BETTER_SMS_FOR_EXPORT s32 Settings::unmountCard() {
    sIsMounted = false;
    return CARDUnmount(sChannel);
}

BETTER_SMS_FOR_EXPORT s32 Settings::saveSettingsGroup(Settings::SettingsGroup &group) {
    CARDFileInfo finfo;
    s32 ret = OpenSavedSettings(group, finfo);
    if (ret < CARD_ERROR_READY) {
        CloseSavedSettings(group, &finfo);
        return ret;
    }

    ret = UpdateSavedSettings(group, &finfo);

    if (ret == CARD_ERROR_READY)
        OSReport("Saved settings for module \"%s\"!\n", Settings::getGroupName(group));
    else {
        OSReport("Failed to save settings for module \"%s\"!\n", Settings::getGroupName(group));
        return ret;
    }

    return CloseSavedSettings(group, &finfo);
}

BETTER_SMS_FOR_EXPORT s32 Settings::loadSettingsGroup(Settings::SettingsGroup &group) {
    CARDFileInfo finfo;

    int ret = OpenSavedSettings(group, finfo);
    if (ret < CARD_ERROR_READY) {
        CloseSavedSettings(group, &finfo);
        return ret;
    }

    // If this returns BROKEN, the save file is desynced by version and should be reset
    if (ReadSavedSettings(group, &finfo) == CARD_ERROR_BROKEN) {
        ret = UpdateSavedSettings(group, &finfo);
        if (ret < CARD_ERROR_READY) {
            CloseSavedSettings(group, &finfo);
            return ret;
        }
    }

    return CloseSavedSettings(group, &finfo);
}

BETTER_SMS_FOR_EXPORT bool Settings::saveAllSettings() {
    TGlobalVector<Settings::SettingsGroup *> groups;
    getSettingsGroups(groups);

    for (auto &group : groups) {
        if (saveSettingsGroup(*group) < CARD_ERROR_READY)
            return false;
    }

    return true;
}

BETTER_SMS_FOR_EXPORT bool Settings::loadAllSettings() {
    TGlobalVector<Settings::SettingsGroup *> groups;
    getSettingsGroups(groups);

    for (auto &group : groups) {
        if (loadSettingsGroup(*group) < CARD_ERROR_READY)
            return false;
    }

    return true;
}

#define DISK_GAME_ID (void *)0x80000000

using namespace BetterSMS;

static Settings::SettingsGroup sSunshineSettingsGroup = {1, 0, Settings::Priority::CORE};
static RumbleSetting sRumbleSetting("Controller Rumble");
static SoundSetting sSoundSetting("Sound Mode");
static SubtitleSetting sSubtitleSetting("Movie Subtitles");

// PRIVATE

void getSettingsGroups(TGlobalVector<Settings::SettingsGroup *> &out) {
    TGlobalVector<Settings::SettingsGroup *> tempCore;
    TGlobalVector<Settings::SettingsGroup *> tempGame;
    TGlobalVector<Settings::SettingsGroup *> tempMode;

    for (auto &item : gModuleInfos) {
        Settings::SettingsGroup *group = item.mSettings;
        if (strcmp(item.mName, "Better Sunshine Engine") == 0) {
            tempCore.insert(tempCore.begin(), group);
            continue;
        }

        switch (group->getPriority()) {
        case Settings::Priority::CORE:
            tempCore.insert(tempCore.end(), group);
            break;
        case Settings::Priority::GAME:
            tempGame.insert(tempGame.end(), group);
            break;
        case Settings::Priority::MODE:
            tempMode.insert(tempMode.end(), group);
            break;
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

BETTER_SMS_FOR_CALLBACK void initAllSettings(TApplication *app) {
    sSunshineSettingsGroup.addSetting(&sRumbleSetting);
    sSunshineSettingsGroup.addSetting(&sSoundSetting);
    sSunshineSettingsGroup.addSetting(&sSubtitleSetting);

    InitCard();
    if (Settings::mountCard() < CARD_ERROR_READY)
        return;

    for (auto &init : gModuleInfos) {
        auto settingsGroup = init.mSettings;
        if (!settingsGroup)  // No settings registered
            continue;

        Settings::loadSettingsGroup(*settingsGroup);
    }

    Settings::unmountCard();
}

void InitCard() { CARDInit(); }

s32 OpenSavedSettings(Settings::SettingsGroup &group, CARDFileInfo &infoOut) {
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

    s32 ret = CARDOpen(sChannel, normalizedPath, &infoOut);
    while (ret == CARD_ERROR_BUSY) {
        ret = CARDCheck(sChannel);
    }

    if (ret == CARD_ERROR_NOFILE) {
        s32 cret =
            CARDCreate(sChannel, normalizedPath, CARD_BLOCKS_TO_BYTES(info.mBlocks), &infoOut);
        // OSReport("Result (CREATE): %d\n", cret);
        if (cret < CARD_ERROR_READY) {
            if (info.mSaveGlobal)
                __CARDSetDiskID(DISK_GAME_ID);
            return cret;
        }
        UpdateSavedSettings(group, &infoOut);
    } else if (ret < CARD_ERROR_READY) {
        if (info.mSaveGlobal)
            __CARDSetDiskID(DISK_GAME_ID);
        return ret;
    }

    // We now have an open handle to the settings file
    return CARD_ERROR_READY;
}

s32 UpdateSavedSettings(Settings::SettingsGroup &group, CARDFileInfo *finfo) {
    auto &info = group.getSaveInfo();

    {
        CARDStat fstatus;

        // Work out status
        int statusRet = CARDGetStatus(finfo->mChannel, finfo->mFileNo, &fstatus);
        if (statusRet < CARD_ERROR_READY)
            return statusRet;

        fstatus.mGameCode = info.mGameCode;
        fstatus.mCompany  = info.mCompany;
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

    const size_t saveDataSize = CARD_BLOCKS_TO_BYTES(info.mBlocks);
    {
        auto saveBuffer    = scoped_ptr<char>(new (JKRHeap::sSystemHeap, 32) char[saveDataSize]);
        auto saveBufferPtr = saveBuffer.get();

        // Reset data
        memset(saveBufferPtr, 0, saveDataSize);

        // Write version info
        saveBufferPtr[0] = group.getMajorVersion();
        saveBufferPtr[1] = group.getMinorVersion();

        // Copy group name into save data
        snprintf(saveBufferPtr + 4, 32, "%s", info.mSaveName);

        // Copy date saved into save data
        {
            OSCalendarTime calendar;
            OSTicksToCalendarTime(OSGetTime(), &calendar);
            snprintf(saveBufferPtr + 36, 32, "Module Info (%lu/%lu/%lu)", calendar.mon + 1,
                     calendar.mday, calendar.year);
        }

        memcpy(saveBufferPtr + CARD_DIRENTRY_SIZE,
               reinterpret_cast<const u8 *>(info.mBannerImage) + info.mBannerImage->mTextureOffset,
               0xE00);
        memcpy(saveBufferPtr + CARD_DIRENTRY_SIZE + 0xE00,
               reinterpret_cast<const u8 *>(info.mIconTable) + info.mIconTable->mTextureOffset,
               0x500 * info.mIconCount);

        size_t dataPosOut = CARD_DIRENTRY_SIZE + 0xE00 + (0x500 * info.mIconCount);

        // Write contents to save file
        JSUMemoryOutputStream out(saveBufferPtr + dataPosOut, saveDataSize - dataPosOut);
        for (auto &setting : group.getSettings()) {
            setting->save(out);
        }

        for (size_t i = 0; i < saveDataSize; i += CARD_BLOCKS_TO_BYTES(1)) {
            s32 result = CARDWrite(finfo, saveBufferPtr, CARD_BLOCKS_TO_BYTES(1), i);
            while (result == CARD_ERROR_BUSY) {
                result = CARDCheck(finfo->mChannel);
            }
            // OSReport("Result (WRITE): %d\n", result);
            if (result < CARD_ERROR_READY) {
                return result;
            }
        }
    }

    return CARD_ERROR_READY;
}

s32 ReadSavedSettings(Settings::SettingsGroup &group, CARDFileInfo *finfo) {
    auto &info = group.getSaveInfo();

    const size_t saveDataSize = CARD_BLOCKS_TO_BYTES(info.mBlocks);
    {
        auto saveBuffer    = scoped_ptr<char>(new (JKRHeap::sSystemHeap, 32) char[saveDataSize]);
        auto saveBufferPtr = saveBuffer.get();

        // Reset data
        memset(saveBufferPtr, 0, saveDataSize);

        for (size_t i = 0; i < saveDataSize; i += CARD_BLOCKS_TO_BYTES(1)) {
            s32 result = CARDRead(finfo, saveBufferPtr, CARD_BLOCKS_TO_BYTES(1), i);
            while (result == CARD_ERROR_BUSY) {
                result = CARDCheck(finfo->mChannel);
            }
            // OSReport("Result (READ): %d\n", result);
            if (result < CARD_ERROR_READY) {
                return result;
            }
        }

        if (saveBufferPtr[0] != group.getMajorVersion()) {
            OSPanic(__FILE__, __LINE__,
                    "Failed to load settings for module \"%s\"! (VERSION MISMATCH)\n\n"
                    "Automatically resetting to defaults...",
                    Settings::getGroupName(group));
            return CARD_ERROR_BROKEN;
        }

        size_t dataPosOut = CARD_DIRENTRY_SIZE + 0xE00 + (0x500 * info.mIconCount);

        // Write contents to save file
        JSUMemoryInputStream in(saveBufferPtr + dataPosOut, saveDataSize - dataPosOut);
        for (auto &setting : group.getSettings()) {
            setting->load(in);
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
        __CARDSetDiskID(DISK_GAME_ID);
    return ret;
}

static TCardBookmarkInfo sBookMarkInfo;

s32 SaveAllSettings() {
    {
        gpCardManager->getBookmarkInfos(&sBookMarkInfo);
        while (gpCardManager->mCommand == TCardManager::GETBOOKMARKS) {
            SMS_ASM_BLOCK("");  // Wait for save to finish
        }

        JSUMemoryOutputStream out(nullptr, 0);
        gpCardManager->getWriteStream(&out);
        TFlagManager::smInstance->save(out);
        gpCardManager->writeBlock(gpApplication.mCurrentSaveBlock);  // This is the block being used
        while (gpCardManager->mCommand == TCardManager::SAVEBLOCK) {
            SMS_ASM_BLOCK("");  // Wait for save to finish
        }
        gpCardManager->unmount();
        TFlagManager::smInstance->saveSuccess();
    }

    {
        s32 cardStatus = Settings::mountCard();
        {
            if (cardStatus < CARD_ERROR_READY) {
                return cardStatus;
            }
            Settings::saveAllSettings();
            Settings::unmountCard();
        }

        gpCardManager->mount_(true);
    }

    {
        JSUMemoryOutputStream out(nullptr, 0);
        gpCardManager->getOptionWriteStream(&out);
        TFlagManager::smInstance->saveOption(out);
        gpCardManager->writeOptionBlock();
    }

    return CARD_ERROR_READY;
}

SettingsDirector::~SettingsDirector() { gpMSound->exitStage(); }

s32 SettingsDirector::direct() {
    s32 ret = 1;

    int *joinBuf[2];

    // mController->read();
    // mController->updateMeaning();
    TSMSFader *fader = gpApplication.mFader;
    if (fader->mFadeStatus == TSMSFader::FADE_OFF) {
        mSettingScreen->mController->mState.mReadInput = true;
    }

    if (mState == State::INIT) {
        if (!OSIsThreadTerminated(&gSetupThread))
            return 0;
        OSJoinThread(&gSetupThread, (void **)joinBuf);

        fader->startFadeinT(0.3f);

        gpMSound->initSound();
        gpMSound->enterStage(MS_WAVE_DOLPIC, 1, 2);
        MSBgm::startBGM(BGM_UNDERGROUND);

        gpCardManager->unmount();

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
        mSettingScreen->mPerformFlags |= 0b0001;    // Disable input
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
    OSCreateThread(&gSetupThread, setupThreadFunc, this, gpSetupThreadStack + 0x10000, 0x10000, 17,
                   0);
    OSResumeThread(&gSetupThread);
}

void *SettingsDirector::setupThreadFunc(void *param) {
    auto *director = reinterpret_cast<SettingsDirector *>(param);
    director->initialize();
    return nullptr;
}

s32 SettingsDirector::exit() {
    TSMSFader *fader = gpApplication.mFader;
    if (fader->mFadeStatus == TSMSFader::FADE_OFF) {
        gpApplication.mFader->startFadeoutT(0.3f);
        gpCardManager->mount_(true);
    }
    return fader->mFadeStatus == TSMSFader::FADE_ON ? 5 : 1;
}

extern BugsSetting gBugFixesSetting;
bool BetterSMS::areBugsPatched() { return gBugFixesSetting.getBool(); }

extern BugsSetting gExploitFixesSetting;
bool BetterSMS::areExploitsPatched() { return gExploitFixesSetting.getBool(); }

extern BugsSetting gCollisionFixesSetting;
bool BetterSMS::isCollisionRepaired() { return gCollisionFixesSetting.getBool(); }

extern Settings::SwitchSetting gCameraInvertXSetting;
bool BetterSMS::isCameraInvertedX() { return gCameraInvertXSetting.getBool(); }

extern Settings::SwitchSetting gCameraInvertYSetting;
bool BetterSMS::isCameraInvertedY() { return gCameraInvertYSetting.getBool(); }

void SettingsDirector::initialize() {
    sRumbleSetting.setBool(TFlagManager::smInstance->getBool(0x90000));
    sSoundSetting.setInt(TFlagManager::smInstance->getFlag(0xA0000));
    sSubtitleSetting.setBool(TFlagManager::smInstance->getBool(0x90001));

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
        mSaveErrorPanel->mPerformFlags |= 0b1011;  // Disable view and input by default
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

        auto *orthoProj                = new JDrama::TOrthoProj();
        orthoProj->mProjectionField[0] = -BetterSMS::getScreenRatioAdjustX();
        orthoProj->mProjectionField[2] = BetterSMS::getScreenRenderWidth();
        screen->assignCamera(orthoProj);

        screen->assignViewObj(groupGrad);

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), screen);
        stageObjGroup->mViewObjList.insert(stageObjGroup->mViewObjList.end(), screen);
    }

    {
        auto *screen = new JDrama::TScreen(screenRect, "Screen 2D");

        auto *orthoProj                = new JDrama::TOrthoProj();
        orthoProj->mProjectionField[0] = -BetterSMS::getScreenRatioAdjustX();
        orthoProj->mProjectionField[2] = BetterSMS::getScreenRenderWidth();
        screen->assignCamera(orthoProj);

        screen->assignViewObj(group2D);

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), screen);
        stageObjGroup->mViewObjList.insert(stageObjGroup->mViewObjList.end(), screen);
    }
}

static size_t newlines(const char *buf) {
    size_t newlineCount = 0;
    for (size_t i = 0; buf[i] != '\0'; ++i) {
        if (buf[i] == '\n')
            newlineCount += 1;
    }
    return newlineCount;
}

void SettingsDirector::initializeSettingsLayout() {
    const int screenOrthoWidth   = BetterSMS::getScreenOrthoWidth();
    const int screenRenderWidth  = BetterSMS::getScreenRenderWidth();
    const int screenRenderHeight = 480;
    const int screenAdjustX      = BetterSMS::getScreenRatioAdjustX();

    mSettingScreen->mScreen =
        new J2DScreen(8, 'ROOT', {0, 0, screenOrthoWidth, screenRenderHeight});
    {
        JUTTexture *mask      = new JUTTexture();
        mask->mTexObj2.val[2] = 0;
        mask->storeTIMG(GetResourceTextureHeader(gMaskBlack));
        mask->_50 = false;

        J2DPicture *maskTop    = new J2DPicture('mskt', {0, 0, 0, 0});
        J2DPicture *maskBottom = new J2DPicture('mskb', {0, 0, 0, 0});

        maskTop->insert(mask, 0, 1.0f);
        maskBottom->insert(mask, 0, 1.0f);

        maskTop->mRect    = {-screenAdjustX, 0, screenOrthoWidth, 90};
        maskBottom->mRect = {-screenAdjustX, screenRenderHeight - 90, screenOrthoWidth,
                             screenRenderHeight};

        maskTop->mAlpha    = 160;
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

        J2DTextBox *label = new J2DTextBox(
            'logo', {0, screenRenderHeight - 90, 600, screenRenderHeight}, gpSystemFont->mFont,
            "Game Settings", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        label->mCharSizeX   = 24;
        label->mCharSizeY   = 24;
        label->mNewlineSize = 24;
        mSettingScreen->mScreen->mChildrenList.append(&label->mPtrLink);

        J2DTextBox *exitLabel = new J2DTextBox(
            'exit',
            {static_cast<int>(20 - getScreenRatioAdjustX()), screenRenderHeight - 90,
             static_cast<int>(100 - getScreenRatioAdjustX()), screenRenderHeight},
            gpSystemFont->mFont, "# Exit", J2DTextBoxHBinding::Left, J2DTextBoxVBinding::Center);
        mSettingScreen->mScreen->mChildrenList.append(&exitLabel->mPtrLink);
    }

    int i = 0;
    TGlobalVector<Settings::SettingsGroup *> settingsGroups;
    getSettingsGroups(settingsGroups);

    settingsGroups.insert(settingsGroups.begin(), &sSunshineSettingsGroup);

    for (auto &group : settingsGroups) {
        auto *groupName = Settings::getGroupName(*group);

        J2DPane *groupPane =
            new J2DPane(19, ('p' << 24) | i, {0, 0, screenRenderWidth, screenRenderHeight});
        groupPane->mIsVisible = false;
        {

            char *groupTextBuf = new char[70];
            memset(groupTextBuf, 0, 70);

            if (settingsGroups.size() == 1) {
                snprintf(groupTextBuf, 70, "    %s    ", groupName);
            } else {
                if (i == 0)
                    snprintf(groupTextBuf, 70, "    %s   >", groupName);
                else if (i == settingsGroups.size() - 1)
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
        groupInfo->mSettingGroup = group;

        int n = 0, ny = 0;
        for (auto &setting : group->getSettings()) {
            if (!setting->isUnlocked())
                continue;

            J2DPane *settingPane =
                new J2DPane(19, ('q' << 24) | i, {0, 0, screenRenderWidth, screenRenderHeight});

            J2DTextBox *settingText = new J2DTextBox(
                ('s' << 24) | n, {0, 110 + (23 * ny), 600, 158 + (23 * ny)}, gpSystemFont->mFont,
                "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);

            J2DTextBox *settingTextBehind = new J2DTextBox(
                ('b' << 24) | n, {2, 112 + (23 * ny), 602, 160 + (23 * ny)}, gpSystemFont->mFont,
                "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
            {
                char valueTextbuf[40];
                setting->getValueName(valueTextbuf);

                char *settingTextBuf = new char[100];
                memset(settingTextBuf, 0, 100);
                snprintf(settingTextBuf, 100, "%s: %s", setting->getName(), valueTextbuf);

                const u8 alpha = setting->isUserEditable() ? 255 : 210;
                const u8 color = setting->isUserEditable() ? 255 : 140;

                settingText->mStrPtr         = settingTextBuf;
                settingText->mCharSizeX      = 21;
                settingText->mCharSizeY      = 21;
                settingText->mNewlineSize    = 21;
                settingText->mGradientBottom = {color, color, color, alpha};
                settingText->mGradientTop    = {color, color, color, alpha};

                settingTextBehind->mStrPtr         = settingTextBuf;
                settingTextBehind->mCharSizeX      = 21;
                settingTextBehind->mCharSizeY      = 21;
                settingTextBehind->mNewlineSize    = 21;
                settingTextBehind->mGradientBottom = {0, 0, 0, alpha};
                settingTextBehind->mGradientTop    = {0, 0, 0, alpha};

                settingPane->mChildrenList.append(&settingTextBehind->mPtrLink);
                settingPane->mChildrenList.append(&settingText->mPtrLink);

                ny += newlines(settingTextBuf) + 1;
            }
            groupPane->mChildrenList.append(&settingPane->mPtrLink);

            auto *settingInfo                = new SettingInfo();
            settingInfo->mSettingTextBox     = settingText;
            settingInfo->mSettingTextBoxBack = settingTextBehind;
            settingInfo->mSettingData        = setting;
            groupInfo->mSettingInfos.insert(groupInfo->mSettingInfos.end(), settingInfo);

            n += 1;
        }

        mSettingScreen->mScreen->mChildrenList.append(&groupPane->mPtrLink);
        mSettingScreen->mGroups.insert(mSettingScreen->mGroups.end(), groupInfo);

        if (i == 0) {
            mSettingScreen->mCurrentGroupInfo   = groupInfo;
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

        J2DPicture *maskPanel = new J2DPicture('mask', {0, 0, 0, 0});
        {
            maskPanel->insert(mask, 0, 1.0f);
            maskPanel->mRect            = {0, 0, 400, 280};
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
                new J2DTextBox('errl', {12, 16, 388, 40}, gpSystemFont->mFont, "",
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
                new J2DTextBox('exit', {80, 230, 202, 258}, gpSystemFont->mFont, "Exit",
                               J2DTextBoxHBinding::Left, J2DTextBoxVBinding::Bottom);
            {
                mSaveErrorPanel->mChoiceBoxes[0]->mCharSizeX = 24;
                mSaveErrorPanel->mChoiceBoxes[0]->mCharSizeY = 24;
            }
            mSaveErrorPanel->mErrorHandlerPane->mChildrenList.append(
                &mSaveErrorPanel->mChoiceBoxes[0]->mPtrLink);

            mSaveErrorPanel->mChoiceBoxes[1] =
                new J2DTextBox('save', {250, 230, 392, 258}, gpSystemFont->mFont, "Retry",
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
                'desc', {20, 50, 380, 230}, gpSystemFont->mFont, "Saving to the memory card...",
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

void SettingsDirector::saveSettings() {
    OSCreateThread(&gSetupThread, saveThreadFunc, this, gpSetupThreadStack + 0x10000, 0x10000, 17,
                   0);
    OSResumeThread(&gSetupThread);
    mState = State::SAVE_BUSY;
}

void SettingsDirector::saveSettings_() {
    char statusBuf[40];
    char messageBuf[100];

    {
        gpCardManager->mount_(true);

        // Save base game settings (language, etc)
        sRumbleSetting.emit();
        sSubtitleSetting.emit();
        sSoundSetting.emit();

        {
            JSUMemoryOutputStream out(nullptr, 0);
            gpCardManager->getOptionWriteStream(&out);
            TFlagManager::smInstance->saveOption(out);
            gpCardManager->writeOptionBlock();
        }

        gpCardManager->unmount();
    }

    {
        s32 cardStatus = Settings::mountCard();

        if (cardStatus < CARD_ERROR_READY) {
            failSave(cardStatus);
            return;
        }

        CARDFileInfo finfo;

        TGlobalVector<Settings::SettingsGroup *> groups;
        getSettingsGroups(groups);

        Settings::saveAllSettings();

        Settings::unmountCard();
    }

    gpCardManager->mount_(true);

    mState     = State::SAVE_SUCCESS;
    mErrorCode = CARD_ERROR_READY;
    return;
}

void SettingsDirector::failSave(int errorcode) {
    Settings::unmountCard();
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
            } else if (fader->mFadeStatus == TSMSFader::FADE_ON) {
                ret = 10;
            }
        }
    }
    return ret;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80299D0C, 0, 0, 0), checkForSettingsMenu);

static J2DScreen *sNotificationScreen;
static J2DTextBox *sNotificationBox;
static TGlobalVector<TGlobalString> sUnlockedSettings;

static OSTime sLastTime  = 0;
static int sVisualState  = 0;
static int sOnScreenTime = 0;

static char sNotifTextBuf[128];

// Settings template specialization for UnorderedMap
using setting_hash = JSystem::hash<BetterSMS::Settings::SingleSetting *>;

struct setting_equal_to : JSystem::binary_function<BetterSMS::Settings::SingleSetting *,
                                                   BetterSMS::Settings::SingleSetting *, bool> {
    inline constexpr auto operator()(BetterSMS::Settings::SingleSetting *a,
                                     BetterSMS::Settings::SingleSetting *b) -> decltype(auto) {
        return a == b;
    }
};

template <>
inline size_t setting_hash::operator()(BetterSMS::Settings::SingleSetting *v) const noexcept {
    auto uv = reinterpret_cast<u32>(v);
    uv      = ((uv >> 16) ^ uv) * 0x45d9f3b;
    uv      = ((uv >> 16) ^ uv) * 0x45d9f3b;
    uv      = (uv >> 16) ^ uv;
    return uv;
}
//

static TGlobalUnorderedMap<Settings::SingleSetting *, bool> sNewUnlockMap(256);

BETTER_SMS_FOR_CALLBACK void initUnlockedSettings(TApplication *app) {
    sLastTime = 0;

    sUnlockedSettings.clear();
    sNewUnlockMap.clear();

    memset(sNotifTextBuf, 0, 128);

    auto *oldHeap = JKRHeap::sRootHeap->becomeCurrentHeap();

    sNotificationScreen = new J2DScreen(8, 'ROOT', {0, 0, 600, 480});
    {
        sNotificationBox = new J2DTextBox(gpSystemFont->mFont, "");
        {
            sNotificationBox->mRect.set(100, 100, 500, 170);
            sNotificationBox->mAlpha       = 0;
            sNotificationBox->mCharSizeX   = 14;
            sNotificationBox->mCharSizeY   = 15;
            sNotificationBox->mNewlineSize = 15;
            sNotificationBox->mHBinding    = J2DTextBoxHBinding::Center;
            sNotificationBox->mVBinding    = J2DTextBoxVBinding::Center;
            sNotificationBox->mStrPtr      = sNotifTextBuf;
        }
        sNotificationScreen->mChildrenList.append(&sNotificationBox->mPtrLink);
    }

    oldHeap->becomeCurrentHeap();

    sVisualState = 0;
}

BETTER_SMS_FOR_CALLBACK void
checkForUnlockedSettings(const Settings::SettingsGroup &group,
                         TGlobalVector<Settings::SingleSetting *> &out) {
    for (auto &setting : group.getSettings()) {
        if (sNewUnlockMap.find(setting) == sNewUnlockMap.end()) {
            sNewUnlockMap[setting] = setting->isUnlocked() && setting->isUserEditable();
        }
        bool &unlocked = sNewUnlockMap[setting];
        if (setting->isUnlocked() && setting->isUserEditable() &&
            !unlocked) {  // This means the setting was just unlocked
            out.insert(out.begin(), setting);
            unlocked = true;
        }
    }
}

BETTER_SMS_FOR_CALLBACK void updateUnlockedSettings(TApplication *app) {
    TGlobalVector<Settings::SettingsGroup *> groups;
    getSettingsGroups(groups);

    for (auto &group : groups) {
        TGlobalVector<Settings::SingleSetting *> unlockedSettings;
        checkForUnlockedSettings(*group, unlockedSettings);

        for (auto &setting : unlockedSettings) {
            char notifbuf[128];
            snprintf(notifbuf, 100, "%s\n\nUnlocked the \"%s\" setting!",
                     Settings::getGroupName(*group), setting->getName());

            sUnlockedSettings.push_back(notifbuf);

            if (sUnlockedSettings.size() == 1) {
                strncpy(sNotificationBox->mStrPtr, notifbuf, 100);
            }
        }
    }
}

BETTER_SMS_FOR_CALLBACK void drawUnlockedSettings(TApplication *app, const J2DOrthoGraph *ortho) {
    if (sUnlockedSettings.size() == 0)
        return;

    ReInitializeGX();
    const_cast<J2DOrthoGraph *>(ortho)->setup2D();

    J2DFillBox(
        sNotificationBox->mRect,
        {30, 70, 230, lerp<u8>(0, 200, static_cast<f32>(sNotificationBox->mAlpha) / 255.0f)});

    sNotificationScreen->draw(0, 0, ortho);

    if (sVisualState == 0) {
        if (sNotificationBox->mAlpha == 0) {
            if (gpMSound->gateCheck(18497))
                MSoundSESystem::MSoundSE::startSoundSystemSE(18497, 0, nullptr, 0);
        }
        sNotificationBox->mAlpha = Min(sNotificationBox->mAlpha + 10, 255);
        if (sNotificationBox->mAlpha == 255)
            sVisualState = 1;
    } else if (sVisualState == 1) {
        sOnScreenTime += 1;
        if (sOnScreenTime > 240) {
            sVisualState  = 2;
            sOnScreenTime = 0;
        }
    } else {
        sNotificationBox->mAlpha = Max(sNotificationBox->mAlpha - 10, 0);
        if (sNotificationBox->mAlpha == 0) {
            sUnlockedSettings.erase(sUnlockedSettings.begin());
            if (sUnlockedSettings.size() > 0) {
                auto notif = sUnlockedSettings.begin();
                strncpy(sNotificationBox->mStrPtr, notif->c_str(), 100);
            }
            sVisualState = 0;
        }
    }
}

#undef DISK_GAME_ID