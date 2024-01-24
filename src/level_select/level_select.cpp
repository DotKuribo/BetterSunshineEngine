#include <JDrama/JDRCamera.hxx>
#include <JDrama/JDRDStageGroup.hxx>
#include <JDrama/JDRDirector.hxx>
#include <JDrama/JDRDisplay.hxx>
#include <JDrama/JDRScreen.hxx>
#include <JGadget/Vector.hxx>
#include <SMS/MSound/MSBGM.hxx>
#include <SMS/MarioUtil/DrawUtil.hxx>
#include <SMS/MarioUtil/gd-reinit-gx.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/System/Resolution.hxx>

#include <J2D/J2DOrthoGraph.hxx>

#include "module.hxx"
#include "p_area.hxx"
#include "p_level_select.hxx"
#include <raw_fn.hxx>

void LevelSelectScreen::perform(u32 flags, JDrama::TGraphics *graphics) {
    if ((flags & 0x1)) {
        processInput();
    }

    if ((flags & 0x3)) {
        u8 areaAlpha = mSelectedAreaID == -1 ? 255 : 0;

        // Reset selection colors
        for (auto &areaInfo : mAreaInfos) {
            areaInfo->mTextBox->mGradientTop       = {255, 255, 255, areaAlpha};
            areaInfo->mTextBox->mGradientBottom    = {255, 255, 255, areaAlpha};
            areaInfo->mEpisodeListPane->mIsVisible = false;
        }

        AreaMenuInfo *curAreaInfo = mAreaInfos[mScrollAreaID];

        // Reset selection colors
        for (auto &episodeInfo : curAreaInfo->mEpisodeInfos) {
            episodeInfo->mTextBox->mGradientTop    = {255, 255, 255, 255};
            episodeInfo->mTextBox->mGradientBottom = {255, 255, 255, 255};
        }

        // Tint the current selection
        curAreaInfo->mTextBox->mGradientTop    = {180, 230, 10, areaAlpha};
        curAreaInfo->mTextBox->mGradientBottom = {240, 170, 10, areaAlpha};

        if (mSelectedAreaID != -1) {
            if (curAreaInfo->mEpisodeInfos.size() > 0) {
                EpisodeMenuInfo *curEpisodeInfo = curAreaInfo->mEpisodeInfos[mScrollEpisodeID];

                // Tint the current selection
                curEpisodeInfo->mTextBox->mGradientTop    = {180, 230, 10, 255};
                curEpisodeInfo->mTextBox->mGradientBottom = {240, 170, 10, 255};
            }

            curAreaInfo->mEpisodeListPane->mIsVisible = true;
        }
    }

    if ((flags & 0x8)) {
        ReInitializeGX();
        SMS_DrawInit();

        J2DOrthoGraph ortho(0, 0, BetterSMS::getScreenOrthoWidth(), SMSGetTitleRenderHeight());
        ortho.setup2D();

        mScreen->draw(0, 0, &ortho);
    }
}

void LevelSelectScreen::processInput() {
    const bool selectingEpisode = mSelectedAreaID != -1;

    // Scroll item
    {
        if ((mController->mButtons.mRapidInput &
             (TMarioGamePad::DPAD_DOWN | TMarioGamePad::MAINSTICK_DOWN))) {
            if (selectingEpisode) {
                mScrollEpisodeID += 1;
            } else {
                mScrollAreaID += 1;
            }
        }

        if ((mController->mButtons.mRapidInput &
             (TMarioGamePad::DPAD_UP | TMarioGamePad::MAINSTICK_UP))) {
            if (selectingEpisode) {
                mScrollEpisodeID -= 1;
            } else {
                mScrollAreaID -= 1;
            }
        }

        if ((mController->mButtons.mRapidInput &
             (TMarioGamePad::DPAD_RIGHT | TMarioGamePad::MAINSTICK_RIGHT))) {
            if (!selectingEpisode) {
                if (mScrollAreaID < mAreaInfos.size() - mColumnSize)
                    mScrollAreaID += mColumnSize;
            }
        }

        if ((mController->mButtons.mRapidInput &
             (TMarioGamePad::DPAD_LEFT | TMarioGamePad::MAINSTICK_LEFT))) {
            if (!selectingEpisode) {
                if (mScrollAreaID >= mColumnSize)
                    mScrollAreaID -= mColumnSize;
            }
        }

        size_t areaCount = mAreaInfos.size();
        if (mScrollAreaID < 0) {
            mScrollAreaID += areaCount;
        } else if (mScrollAreaID >= areaCount) {
            mScrollAreaID -= areaCount;
        }

        if (selectingEpisode) {
            size_t episodeCount = mAreaInfos[mSelectedAreaID]->mEpisodeInfos.size();
            if (mScrollEpisodeID < 0) {
                mScrollEpisodeID += episodeCount;
            } else if (mScrollEpisodeID >= episodeCount) {
                mScrollEpisodeID -= episodeCount;
            }
        }
    }

    // Select item
    {
        if ((mController->mButtons.mFrameInput & TMarioGamePad::A)) {
            if (selectingEpisode) {
                // Tell director to load the scene
                mSelectedEpisodeID = mScrollEpisodeID;
                mShouldExit        = true;
            } else {
                mSelectedAreaID = mScrollAreaID;
            }
        }

        if ((mController->mButtons.mFrameInput & TMarioGamePad::B)) {
            if (selectingEpisode) {
                mSelectedAreaID    = -1;
                mSelectedEpisodeID = -1;
            } else {
                mShouldExit = true;
            }
        }
    }
}

AreaMenuInfo *LevelSelectScreen::getAreaInfo(u32 index) {
    if (index >= mAreaInfos.size())
        return nullptr;

    return mAreaInfos.at(index);
}

EpisodeMenuInfo *LevelSelectScreen::getEpisodeInfo(u32 index) {
    if (mSelectedAreaID == -1)
        return nullptr;

    const AreaMenuInfo *info = mAreaInfos[mSelectedAreaID];

    if (index >= info->mEpisodeInfos.size())
        return nullptr;

    return info->mEpisodeInfos.at(index);
}

void *LevelSelectDirector::setupThreadFunc(void *param) {
    auto *director = reinterpret_cast<LevelSelectDirector *>(param);
    director->initialize();
    return nullptr;
}

void LevelSelectDirector::setup(JDrama::TDisplay *display, TMarioGamePad *controller) {
    mViewObjStageGroup             = new JDrama::TDStageGroup(display);
    mDisplay                       = display;
    mController                    = controller;
    mController->mState.mReadInput = false;
    OSCreateThread(&gSetupThread, setupThreadFunc, this, gpSetupThreadStack + 0x10000, 0x10000, 17,
                   0);
    OSResumeThread(&gSetupThread);
}

void LevelSelectDirector::initialize() {
    initializeDramaHierarchy();
    initializeLevelsLayout();
}

void LevelSelectDirector::initializeDramaHierarchy() {
    auto *stageObjGroup = reinterpret_cast<JDrama::TDStageGroup *>(mViewObjStageGroup);
    auto *rootObjGroup  = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Root View Objs");
    mViewObjRoot        = rootObjGroup;

    JDrama::TRect screenRect{0, 0, SMSGetTitleRenderWidth(), SMSGetTitleRenderHeight()};

    auto *group2D = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Group 2D");
    {
        mSelectScreen = new LevelSelectScreen(mController);
        group2D->mViewObjList.insert(group2D->mViewObjList.end(), mSelectScreen);

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), group2D);
    }

    // auto *groupGrad = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Group Grad");
    //{
    //     mGradBG = new TSelectGrad("<TSelectGrad>");
    //     mGradBG->setStageColor(1);

    //    groupGrad->mViewObjList.insert(groupGrad->mViewObjList.end(), mGradBG);

    //    rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), groupGrad);
    //}

    {
        auto *stageDisp = new JDrama::TDStageDisp("<DStageDisp>", {0});

        auto *efbCtrl = stageDisp->getEfbCtrlDisp();
        efbCtrl->setSrcRect(screenRect);

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), stageDisp);
        stageObjGroup->mViewObjList.insert(stageObjGroup->mViewObjList.end(), stageDisp);
    }

    //{
    //    auto *screen = new JDrama::TScreen(screenRect, "Screen Grad");

    //    auto *orthoProj                = new JDrama::TOrthoProj();
    //    orthoProj->mProjectionField[0] = -BetterSMS::getScreenRatioAdjustX();
    //    orthoProj->mProjectionField[2] = BetterSMS::getScreenRenderWidth();
    //    screen->assignCamera(orthoProj);

    //    screen->assignViewObj(groupGrad);

    //    rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), screen);
    //    stageObjGroup->mViewObjList.insert(stageObjGroup->mViewObjList.end(), screen);
    //}

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

void LevelSelectDirector::initializeLevelsLayout() {
    const int screenOrthoWidth   = BetterSMS::getScreenOrthoWidth();
    const int screenRenderWidth  = BetterSMS::getScreenRenderWidth();
    const int screenRenderHeight = 480;
    const int screenAdjustX      = BetterSMS::getScreenRatioAdjustX();

    void *stageNameData = JKRFileLoader::getGlbResource("/common/2d/stagename.bmg");
    SMS_ASSERT(stageNameData, "Missing /common/2d/stagename.bmg!");

    void *scenarioNameData = JKRFileLoader::getGlbResource("/common/2d/scenarioname.bmg");
    SMS_ASSERT(scenarioNameData, "Missing /common/2d/scenarioname.bmg!");

    mSelectScreen->mScreen = new J2DScreen(8, 'ROOT', {0, 0, screenOrthoWidth, screenRenderHeight});
    {
        J2DTextBox *label = new J2DTextBox(
            'logo', {0, screenRenderHeight - 90, 600, screenRenderHeight}, gpSystemFont->mFont,
            "Level Select", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        label->mCharSizeX   = 24;
        label->mCharSizeY   = 24;
        label->mNewlineSize = 24;
        mSelectScreen->mScreen->mChildrenList.append(&label->mPtrLink);

        J2DTextBox *exitLabel = new J2DTextBox(
            'exit',
            {static_cast<int>(20 - BetterSMS::getScreenRatioAdjustX()), screenRenderHeight - 90,
             static_cast<int>(100 - BetterSMS::getScreenRatioAdjustX()), screenRenderHeight},
            gpSystemFont->mFont, "# Exit", J2DTextBoxHBinding::Left, J2DTextBoxVBinding::Center);
        mSelectScreen->mScreen->mChildrenList.append(&exitLabel->mPtrLink);
    }

    BetterSMS::Stage::AreaInfo **areaInfos = BetterSMS::Stage::getAreaInfos();

    size_t areaCount = 0;
    for (s32 i = 0; i < BETTER_SMS_AREA_MAX; ++i) {
        if (!areaInfos[i]) {
            continue;
        }
        areaCount += 1;
    }

    size_t rowsPerColumn = 14;
    size_t columns       = (areaCount / rowsPerColumn) + 1;
    size_t areaFontSize  = 21 - (4 * (columns - 1));

    mSelectScreen->mColumnSize  = rowsPerColumn;
    mSelectScreen->mColumnCount = columns;

    s32 flatRow = 0;
    for (s32 i = 0; i < BETTER_SMS_AREA_MAX; ++i) {
        auto *info = areaInfos[i];
        if (!info) {
            OSReport("Skipping area index %d\n", i);
            continue;
        }

        const char *stageName = nullptr;

        // Pop up episode list pane
        J2DPane *areaPane    = new J2DPane(19, ('s' << 24) | info->mNormalStageID,
                                           {0, 0, screenRenderWidth, screenRenderHeight});
        areaPane->mIsVisible = false;
        {
            char *groupTextBuf = new char[64];
            memset(groupTextBuf, 0, 64);

            stageName = (const char *)SMSGetMessageData__FPvUl(stageNameData, info->mShineStageID);
            SMS_ASSERT(stageName, "Missing stage name for area ID %d (%X)", info->mShineStageID,
                       info->mShineStageID);

            snprintf(groupTextBuf, 64, "%s", stageName);

            J2DTextBox *label = new J2DTextBox(
                ('l' << 24) | info->mNormalStageID, {0, 0, 600, 90}, gpSystemFont->mFont,
                groupTextBuf, J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
            label->mCharSizeX   = 24;
            label->mCharSizeY   = 24;
            label->mNewlineSize = 24;
            areaPane->mChildrenList.append(&label->mPtrLink);
        }

        OSReport("Stagename %d: %s\n", i, stageName);

        int textWidth = 560 / columns;
        int textX     = 20 + (flatRow / rowsPerColumn) * textWidth;
        int textY     = 50 + (flatRow % rowsPerColumn) * (areaFontSize + 2);

        // Area listing
        J2DTextBox *areaText = new J2DTextBox(
            ('a' << 24) | info->mNormalStageID, {textX, textY, textX + textWidth, textY + 48},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Left, J2DTextBoxVBinding::Center);
        {
            char *areaTextBuf = new char[100];
            memset(areaTextBuf, 0, 100);
            snprintf(areaTextBuf, 100, "%s", stageName);

            size_t nameLen           = strlen(areaTextBuf);
            size_t adjustedFontWidth = nameLen > 16 ? areaFontSize - (nameLen - 16) : areaFontSize;

            areaText->mStrPtr         = areaTextBuf;
            areaText->mCharSizeX      = adjustedFontWidth;
            areaText->mCharSizeY      = areaFontSize;
            areaText->mNewlineSize    = areaFontSize;
            areaText->mGradientBottom = {255, 255, 255, 255};
            areaText->mGradientTop    = {255, 255, 255, 255};
        }
        mSelectScreen->mScreen->mChildrenList.append(&areaText->mPtrLink);

        AreaMenuInfo *areaMenuInfo     = new AreaMenuInfo();
        areaMenuInfo->mEpisodeListPane = areaPane;
        areaMenuInfo->mTextBox         = areaText;
        areaMenuInfo->mStageID         = info->mNormalStageID;

        s32 en = 0, eny = 0;
        for (s32 j = 0; j < info->mScenarioNameIDs.size(); ++j) {
            if (info->mScenarioIDs.size() != info->mScenarioNameIDs.size()) {
                OSReport("[WARNING] Scenario count mismatches name count! %lu / %lu\n",
                         info->mScenarioIDs.size(), info->mScenarioNameIDs.size());
            }

            u8 scenarioID      = info->mScenarioIDs[j];
            s32 scenarioNameID = info->mScenarioNameIDs[j];

            OSReport("[%s] Index %d, ID: %u, Name ID: %d\n", stageName, j, scenarioID,
                     scenarioNameID);

            J2DTextBox *episodeText = new J2DTextBox(
                ('e' << 24) | scenarioID, {0, 110 + (23 * eny), 600, 158 + (23 * eny)},
                gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
            {
                const char *scenarioName = (const char *)SMSGetMessageData__FPvUl(
                    scenarioNameData, info->mScenarioNameIDs[j]);
                SMS_ASSERT(scenarioName,
                           "Missing episode name for scenario ID %u (%X) [name ID %d (%X)]",
                           scenarioID, scenarioID, scenarioNameID, scenarioNameID);

                char *episodeTextBuf = new char[100];
                memset(episodeTextBuf, 0, 100);
                snprintf(episodeTextBuf, 100, "%s", scenarioName);

                episodeText->mStrPtr         = episodeTextBuf;
                episodeText->mCharSizeX      = 21;
                episodeText->mCharSizeY      = 21;
                episodeText->mNewlineSize    = 21;
                episodeText->mGradientBottom = {255, 255, 255, 255};
                episodeText->mGradientTop    = {255, 255, 255, 255};

                eny += 1;
            }
            areaPane->mChildrenList.append(&episodeText->mPtrLink);

            EpisodeMenuInfo *episodeInfo = new EpisodeMenuInfo();
            episodeInfo->mTextBox        = episodeText;
            episodeInfo->mScenarioID     = scenarioID;
            areaMenuInfo->mEpisodeInfos.insert(areaMenuInfo->mEpisodeInfos.end(), episodeInfo);

            en += 1;
        }

        mSelectScreen->mScreen->mChildrenList.append(&areaPane->mPtrLink);
        mSelectScreen->mAreaInfos.insert(mSelectScreen->mAreaInfos.end(), areaMenuInfo);

        flatRow += 1;
    }
}

s32 LevelSelectDirector::direct() {
    s32 ret = 1;

    int *joinBuf[2];

    // mController->read();
    // mController->updateMeaning();
    TSMSFader *fader = gpApplication.mFader;
    if (fader->mFadeStatus == TSMSFader::FADE_OFF) {
        mSelectScreen->mController->mState.mReadInput = false;
        mSelectScreen->mController->mState._02        = true;
    }

    if (mState == State::INIT) {
        if (!OSIsThreadTerminated(&gSetupThread))
            return 0;
        OSJoinThread(&gSetupThread, (void **)joinBuf);

        fader->startFadeinT(0.3f);

        gpMSound->initSound();
        gpMSound->enterStage((MS_SCENE_WAVE)517, 10, 0);
        MSBgm::startBGM(BGM_MARE_SEA);

        mState = State::CONTROL;
        return 0;
    }

    TDirector::direct();

    switch (mState) {
    case State::INIT:
        break;
    case State::CONTROL:
        mSelectScreen->mPerformFlags &= ~0b0001;  // Enable input by default;

        // The area and episode have been selected, enter the stage.
        if (mSelectScreen->mShouldExit) {
            if (mSelectScreen->mSelectedAreaID != -1 && mSelectScreen->mSelectedEpisodeID != -1) {
                gpApplication.mNextScene.mAreaID =
                    mSelectScreen->mAreaInfos[mSelectScreen->mSelectedAreaID]->mStageID;
                gpApplication.mNextScene.mEpisodeID = mSelectScreen->mSelectedEpisodeID;
            }
            mState                              = State::EXIT;
        }
        break;
    case State::EXIT: {
        ret = exit();
        break;
    }
    }
    return ret;
}

s32 LevelSelectDirector::exit() {
    TSMSFader *fader = gpApplication.mFader;
    if (fader->mFadeStatus == TSMSFader::FADE_OFF) {
        gpApplication.mFader->startFadeoutT(0.3f);
        MSBgm::stopBGM(BGM_MARE_SEA, 10);
    }
    return fader->mFadeStatus == TSMSFader::FADE_ON ? 5 : 1;
}