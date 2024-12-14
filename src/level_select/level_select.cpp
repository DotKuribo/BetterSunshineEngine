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

#include "game.hxx"
#include "module.hxx"
#include "p_area.hxx"
#include "p_level_select.hxx"
#include "stage.hxx"
#include <DVD.h>
#include <raw_fn.hxx>
#include "loading.hxx"

#define TEXT_COLOR_DEFAULT_SCENARIO                                                                \
    { 255, 255, 255, 255 }
#define TEXT_COLOR_DEFAULT_FILENAME                                                                \
    { 200, 255, 200, 255 }
#define TEXT_COLOR_TOP_SELECTED                                                                    \
    { 180, 230, 10, 255 }
#define TEXT_COLOR_BOTTOM_SELECTED                                                                 \
    { 240, 170, 10, 255 }

static bool sceneExists(u32 areaID, u32 episodeID) {
    if (areaID >= gpApplication.mStageArchiveAry->mChildren.size()) {
        OSReport("Area ID %d NOT FOUND\n", areaID);
        return false;
    }

    auto *areaInfo = reinterpret_cast<TNameRefAryT<TScenarioArchiveName> *>(
        gpApplication.mStageArchiveAry->mChildren[areaID]);

    if (episodeID >= areaInfo->mChildren.size()) {
        OSReport("Area ID %d, Episode ID %d NOT FOUND\n", areaID, episodeID);
        return false;
    }

    TScenarioArchiveName episodeInfo = areaInfo->mChildren[episodeID];
    char stageName[128];
    snprintf(stageName, 128, "/data/scene/%s", episodeInfo.mArchiveName);
    char *loc = strstr(stageName, ".arc");
    if (loc) {
        strncpy(loc, ".szs", 4);
    }

    if (DVDConvertPathToEntrynum(stageName) >= 0) {
        return true;
    } else {
        OSReport("Area ID %d, Episode ID %d, Name %s NOT FOUND\n", areaID, episodeID, stageName);
        return false;
    }
}

static bool sceneFilename(char *out, size_t buf_size, u32 areaID, u32 episodeID) {
    if (areaID >= gpApplication.mStageArchiveAry->mChildren.size()) {
        OSReport("Area ID %d NOT FOUND\n", areaID);
        return false;
    }

    auto *areaInfo = reinterpret_cast<TNameRefAryT<TScenarioArchiveName> *>(
        gpApplication.mStageArchiveAry->mChildren[areaID]);

    if (episodeID >= areaInfo->mChildren.size()) {
        OSReport("Area ID %d, Episode ID %d NOT FOUND\n", areaID, episodeID);
        return false;
    }

    TScenarioArchiveName episodeInfo = areaInfo->mChildren[episodeID];
    char stageName[128];
    snprintf(stageName, 128, "/data/scene/%s", episodeInfo.mArchiveName);
    char *loc = strstr(stageName, ".arc");
    if (loc) {
        strncpy(loc, ".szs", 4);
    }

    if (DVDConvertPathToEntrynum(stageName) >= 0) {
        snprintf(out, buf_size, "%s", episodeInfo.mArchiveName);
        char *loc = strstr(out, ".arc");
        if (loc) {
            strncpy(loc, ".szs", 4);
        }
        return true;
    } else {
        OSReport("Area ID %d, Episode ID %d, Name %s NOT FOUND\n", areaID, episodeID, stageName);
        return false;
    }
}

void LevelSelectScreen::perform(u32 flags, JDrama::TGraphics *graphics) {
    if ((flags & 0x1)) {
        processInput();
    }

    if ((flags & 0x3)) {
        u8 areaAlpha = mSelectedAreaID == -1 ? 255 : 0;

        // Reset selection colors
        for (auto &areaInfo : mAreaInfos) {
            areaInfo->mTextBox->mGradientTop       = TEXT_COLOR_DEFAULT_SCENARIO;
            areaInfo->mTextBox->mGradientBottom    = TEXT_COLOR_DEFAULT_SCENARIO;
            areaInfo->mTextBox->mGradientTop.a     = areaAlpha;
            areaInfo->mTextBox->mGradientBottom.a  = areaAlpha;
            areaInfo->mEpisodeListPane->mIsVisible = false;
        }

        AreaMenuInfo *curAreaInfo = mAreaInfos[mScrollAreaID];

        // Reset selection colors
        for (auto &episodeInfo : curAreaInfo->mEpisodeInfos) {
            episodeInfo->mScenarioTextBox->mGradientTop    = TEXT_COLOR_DEFAULT_SCENARIO;
            episodeInfo->mScenarioTextBox->mGradientBottom = TEXT_COLOR_DEFAULT_SCENARIO;
            episodeInfo->mFilenameTextBox->mGradientTop    = TEXT_COLOR_DEFAULT_FILENAME;
            episodeInfo->mFilenameTextBox->mGradientBottom = TEXT_COLOR_DEFAULT_FILENAME;
        }

        // Tint the current selection
        curAreaInfo->mTextBox->mGradientTop      = TEXT_COLOR_TOP_SELECTED;
        curAreaInfo->mTextBox->mGradientBottom   = TEXT_COLOR_BOTTOM_SELECTED;
        curAreaInfo->mTextBox->mGradientTop.a    = areaAlpha;
        curAreaInfo->mTextBox->mGradientBottom.a = areaAlpha;

        if (mSelectedAreaID != -1) {
            if (curAreaInfo->mEpisodeInfos.size() > 0) {
                EpisodeMenuInfo *curEpisodeInfo = curAreaInfo->mEpisodeInfos[mScrollEpisodeID];

                // Tint the current selection
                curEpisodeInfo->mScenarioTextBox->mGradientTop    = TEXT_COLOR_TOP_SELECTED;
                curEpisodeInfo->mScenarioTextBox->mGradientBottom = TEXT_COLOR_BOTTOM_SELECTED;
                curEpisodeInfo->mFilenameTextBox->mGradientTop    = TEXT_COLOR_TOP_SELECTED;
                curEpisodeInfo->mFilenameTextBox->mGradientBottom = TEXT_COLOR_BOTTOM_SELECTED;
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

    // Check for filename select
    if ((mController->mButtons.mInput & TMarioGamePad::Z)) {
        for (auto &areaInfo : mAreaInfos) {
            for (auto &episodeInfo : areaInfo->mEpisodeInfos) {
                if (episodeInfo->mScenarioTextBox) {
                    episodeInfo->mScenarioTextBox->mIsVisible = false;
                }
                if (episodeInfo->mFilenameTextBox) {
                    episodeInfo->mFilenameTextBox->mIsVisible = true;
                }
            }
        }
    } else {
        for (auto &areaInfo : mAreaInfos) {
            for (auto &episodeInfo : areaInfo->mEpisodeInfos) {
                if (episodeInfo->mScenarioTextBox) {
                    episodeInfo->mScenarioTextBox->mIsVisible = true;
                }
                if (episodeInfo->mFilenameTextBox) {
                    episodeInfo->mFilenameTextBox->mIsVisible = false;
                }
            }
        }
    }

    // Select item
    {
        if ((mController->mButtons.mFrameInput & TMarioGamePad::A)) {
            if (selectingEpisode) {
                // Check if there are any episodes to select
                if (mAreaInfos[mSelectedAreaID]->mEpisodeInfos.size() > 0) {
                    // Tell director to load the scene
                    mSelectedEpisodeID = mScrollEpisodeID;
                    mShouldExit        = true;
                }
            } else {
                mSelectedAreaID  = mScrollAreaID;
                mScrollEpisodeID = 0;
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

bool LevelSelectScreen::genAreaText(s32 flatRow, u8 normalStageID, u8 shineStageID,
                                    void *stageNameData, void *scenarioNameData) {
    const int screenRenderWidth  = BetterSMS::getScreenRenderWidth();
    const int screenRenderHeight = 480;

    size_t areaFontSize = 21 - (4 * (mColumnCount - 1));

    const char *stageName = (const char *)SMSGetMessageData__FPvUl(stageNameData, shineStageID);
    SMS_ASSERT(stageName, "Missing stage name for area ID %d (%X)", shineStageID, shineStageID);

    AreaMenuInfo *areaMenuInfo;

    // Pop up episode list pane
    bool created;
    J2DPane *areaPane =
        findOrCreateAreaPane(normalStageID, shineStageID, screenRenderWidth, screenRenderHeight, &created);
    if (!created) {
        return false;
    }

    {
        char *groupTextBuf = new char[64];
        memset(groupTextBuf, 0, 64);

        snprintf(groupTextBuf, 64, "%s", stageName);

        J2DTextBox *label =
            new J2DTextBox(('l' << 24) | normalStageID, {0, 30, 600, 120}, gpSystemFont->mFont,
                           groupTextBuf, J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        label->mCharSizeX      = 26;
        label->mCharSizeY      = 26;
        label->mNewlineSize    = 26;
        label->mGradientTop    = {240, 10, 170, 255};
        label->mGradientBottom = {180, 10, 230, 255};
        areaPane->mChildrenList.append(&label->mPtrLink);
    }

    int textWidth = 500 / mColumnCount;
    int textX     = 50 + (flatRow / mColumnSize) * (textWidth + 4);
    int textY     = 70 + (flatRow % mColumnSize) * (areaFontSize + 2);

    // Area listing
    J2DTextBox *areaText = new J2DTextBox(
        ('a' << 24) | normalStageID, {textX, textY, textX + textWidth, textY + 48},
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
        areaText->mGradientBottom = TEXT_COLOR_DEFAULT_SCENARIO;
        areaText->mGradientTop    = TEXT_COLOR_DEFAULT_SCENARIO;
    }
    mScreen->mChildrenList.append(&areaText->mPtrLink);

    areaMenuInfo                   = new AreaMenuInfo();
    areaMenuInfo->mEpisodeListPane = areaPane;
    areaMenuInfo->mTextBox         = areaText;
    areaMenuInfo->mStageID         = shineStageID;
    mAreaInfos.insert(mAreaInfos.end(), areaMenuInfo);

    if (flatRow == 1) {
        genEpisodeTextDelfinoPlaza(*areaMenuInfo, normalStageID, shineStageID, scenarioNameData);
    } else {
        genEpisodeText(*areaMenuInfo, normalStageID, shineStageID, scenarioNameData);
    }

    mScreen->mChildrenList.append(&areaPane->mPtrLink);
    return true;
}

bool LevelSelectScreen::genAreaTextTest1(s32 flatRow) {
    const int screenRenderWidth  = BetterSMS::getScreenRenderWidth();
    const int screenRenderHeight = 480;

    size_t areaFontSize = 21 - (4 * (mColumnCount - 1));

    const char *stageName = nullptr;

    // Pop up episode list pane
    J2DPane *areaPane    = new J2DPane(19, ('s' << 24) | ('t' << 16) | ('1' << 8),
                                       {0, 0, screenRenderWidth, screenRenderHeight});
    areaPane->mIsVisible = false;
    {
        char *groupTextBuf = new char[64];
        memset(groupTextBuf, 0, 64);

        stageName = "TEST MAP 1X";
        snprintf(groupTextBuf, 64, "%s", stageName);

        J2DTextBox *label      = new J2DTextBox(('l' << 24) | ('t' << 16) | ('1' << 8),
                                                {0, 30, 600, 120}, gpSystemFont->mFont, groupTextBuf,
                                                J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        label->mCharSizeX      = 26;
        label->mCharSizeY      = 26;
        label->mNewlineSize    = 26;
        label->mGradientTop    = {240, 10, 170, 255};
        label->mGradientBottom = {180, 10, 230, 255};
        areaPane->mChildrenList.append(&label->mPtrLink);
    }

    int textWidth = 500 / mColumnCount;
    int textX     = 50 + (flatRow / mColumnSize) * (textWidth + 4);
    int textY     = 70 + (flatRow % mColumnSize) * (areaFontSize + 2);

    // Area listing
    J2DTextBox *areaText = new J2DTextBox(
        ('a' << 24) | ('t' << 16) | ('1' << 8), {textX, textY, textX + textWidth, textY + 48},
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
        areaText->mGradientBottom = TEXT_COLOR_DEFAULT_SCENARIO;
        areaText->mGradientTop    = TEXT_COLOR_DEFAULT_SCENARIO;
    }
    mScreen->mChildrenList.append(&areaText->mPtrLink);

    AreaMenuInfo *areaMenuInfo     = new AreaMenuInfo();
    areaMenuInfo->mEpisodeListPane = areaPane;
    areaMenuInfo->mTextBox         = areaText;
    areaMenuInfo->mStageID         = 12;
    genEpisodeTextTest1(*areaMenuInfo);

    mScreen->mChildrenList.append(&areaPane->mPtrLink);
    mAreaInfos.insert(mAreaInfos.end(), areaMenuInfo);

    return true;
}

bool LevelSelectScreen::genAreaTextTest2(s32 flatRow) {
    const int screenRenderWidth  = BetterSMS::getScreenRenderWidth();
    const int screenRenderHeight = 480;

    size_t areaFontSize = 21 - (4 * (mColumnCount - 1));

    const char *stageName = nullptr;

    // Pop up episode list pane
    J2DPane *areaPane    = new J2DPane(19, ('s' << 24) | ('t' << 16) | ('1' << 8),
                                       {0, 0, screenRenderWidth, screenRenderHeight});
    areaPane->mIsVisible = false;
    {
        char *groupTextBuf = new char[64];
        memset(groupTextBuf, 0, 64);

        stageName = "TEST MAP 2X";
        snprintf(groupTextBuf, 64, "%s", stageName);

        J2DTextBox *label      = new J2DTextBox(('l' << 24) | ('t' << 16) | ('1' << 8),
                                                {0, 30, 600, 120}, gpSystemFont->mFont, groupTextBuf,
                                                J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        label->mCharSizeX      = 26;
        label->mCharSizeY      = 26;
        label->mNewlineSize    = 26;
        label->mGradientTop    = {240, 10, 170, 255};
        label->mGradientBottom = {180, 10, 230, 255};
        areaPane->mChildrenList.append(&label->mPtrLink);
    }

    int textWidth = 500 / mColumnCount;
    int textX     = 50 + (flatRow / mColumnSize) * (textWidth + 4);
    int textY     = 70 + (flatRow % mColumnSize) * (areaFontSize + 2);

    // Area listing
    J2DTextBox *areaText = new J2DTextBox(
        ('a' << 24) | ('t' << 16) | ('1' << 8), {textX, textY, textX + textWidth, textY + 48},
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
        areaText->mGradientBottom = TEXT_COLOR_DEFAULT_SCENARIO;
        areaText->mGradientTop    = TEXT_COLOR_DEFAULT_SCENARIO;
    }
    mScreen->mChildrenList.append(&areaText->mPtrLink);

    AreaMenuInfo *areaMenuInfo     = new AreaMenuInfo();
    areaMenuInfo->mEpisodeListPane = areaPane;
    areaMenuInfo->mTextBox         = areaText;
    areaMenuInfo->mStageID         = 17;
    genEpisodeTextTest2(*areaMenuInfo);

    mScreen->mChildrenList.append(&areaPane->mPtrLink);
    mAreaInfos.insert(mAreaInfos.end(), areaMenuInfo);

    return true;
}

bool LevelSelectScreen::genAreaTextScale(s32 flatRow) {
    const int screenRenderWidth  = BetterSMS::getScreenRenderWidth();
    const int screenRenderHeight = 480;

    size_t areaFontSize = 21 - (4 * (mColumnCount - 1));

    const char *stageName = nullptr;

    // Pop up episode list pane
    J2DPane *areaPane    = new J2DPane(19, ('s' << 24) | ('t' << 16) | ('1' << 8),
                                       {0, 0, screenRenderWidth, screenRenderHeight});
    areaPane->mIsVisible = false;
    {
        char *groupTextBuf = new char[64];
        memset(groupTextBuf, 0, 64);

        stageName = "SCALE MAP";
        snprintf(groupTextBuf, 64, "%s", stageName);

        J2DTextBox *label      = new J2DTextBox(('l' << 24) | ('t' << 16) | ('1' << 8),
                                                {0, 30, 600, 120}, gpSystemFont->mFont, groupTextBuf,
                                                J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        label->mCharSizeX      = 26;
        label->mCharSizeY      = 26;
        label->mNewlineSize    = 26;
        label->mGradientTop    = {240, 10, 170, 255};
        label->mGradientBottom = {180, 10, 230, 255};
        areaPane->mChildrenList.append(&label->mPtrLink);
    }

    int textWidth = 500 / mColumnCount;
    int textX     = 50 + (flatRow / mColumnSize) * (textWidth + 4);
    int textY     = 70 + (flatRow % mColumnSize) * (areaFontSize + 2);

    // Area listing
    J2DTextBox *areaText = new J2DTextBox(
        ('a' << 24) | ('t' << 16) | ('1' << 8), {textX, textY, textX + textWidth, textY + 48},
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
        areaText->mGradientBottom = TEXT_COLOR_DEFAULT_SCENARIO;
        areaText->mGradientTop    = TEXT_COLOR_DEFAULT_SCENARIO;
    }
    mScreen->mChildrenList.append(&areaText->mPtrLink);

    AreaMenuInfo *areaMenuInfo     = new AreaMenuInfo();
    areaMenuInfo->mEpisodeListPane = areaPane;
    areaMenuInfo->mTextBox         = areaText;
    areaMenuInfo->mStageID         = 11;
    genEpisodeTextScale(*areaMenuInfo);

    mScreen->mChildrenList.append(&areaPane->mPtrLink);
    mAreaInfos.insert(mAreaInfos.end(), areaMenuInfo);

    return true;
}

void LevelSelectScreen::genEpisodeText(AreaMenuInfo &menu, u8 normalStageID, u8 shineStageID,
                                       void *scenarioNameData) {
    const Stage::ShineAreaInfo *info = Stage::getShineAreaInfos()[shineStageID];
    if (!info) {
        return;
    }

    const TGlobalVector<s32> &scenarioIDs = info->getScenarioIDs();

    size_t rows = 0;
    for (s32 j = 0; j < scenarioIDs.size(); ++j) {
        if (!sceneExists(normalStageID, j)) {
            continue;
        }
        rows += 1;
    }

    for (s32 j = 0; j < BETTER_SMS_EXAREA_MAX; ++j) {
        const Stage::ExAreaInfo &exinfo = Stage::getExAreaInfos()[j];
        if (exinfo.mShineStageID == -1) {
            continue;
        }

        if (shineStageID != exinfo.mShineStageID) {
            continue;
        }

        if (!sceneExists(j, 0)) {
            continue;
        }

        rows += 1;
    }

    size_t textBaseHeight = 21;
    size_t textHeight     = rows < 10 ? textBaseHeight : (textBaseHeight - (rows - 10));

    const TGlobalVector<s32> &scenarioNameIDs = info->getScenarioNameIDs();

    s32 en = 0, eny = 0;
    for (s32 j = 0; j < scenarioIDs.size(); ++j) {
        if (scenarioIDs.size() > scenarioNameIDs.size()) {
            OSReport("[WARNING] Scenario count mismatches name count! %lu / %lu\n",
                     scenarioIDs.size(), scenarioNameIDs.size());
        }

        char filename[128];
        if (!sceneFilename(filename, 128, normalStageID, j)) {
            continue;
        }

        u8 scenarioID      = scenarioIDs[j];
        s32 scenarioNameID = j >= scenarioNameIDs.size() ? -1 : scenarioNameIDs[j];

        J2DTextBox *scenarioText = new J2DTextBox(
            ('e' << 24) | scenarioNameID & 0xFFFFFF,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *scenarioTextBuf = new char[100];
            memset(scenarioTextBuf, 0, 100);

            if (scenarioNameID != -1) {
                const char *scenarioName =
                    (const char *)SMSGetMessageData__FPvUl(scenarioNameData, scenarioNameID);
                SMS_ASSERT(scenarioName,
                           "Missing scenario name for scenario ID %u (%X) [name ID %d (%X)]",
                           scenarioID, scenarioID, scenarioNameID, scenarioNameID);
                snprintf(scenarioTextBuf, 100, "%s", scenarioName);
            } else {
                snprintf(scenarioTextBuf, 100, "Scenario %ld", j);
            }

            scenarioText->mStrPtr         = scenarioTextBuf;
            scenarioText->mCharSizeX      = 21;
            scenarioText->mCharSizeY      = textHeight;
            scenarioText->mNewlineSize    = textHeight;
            scenarioText->mGradientBottom = TEXT_COLOR_DEFAULT_SCENARIO;
            scenarioText->mGradientTop    = TEXT_COLOR_DEFAULT_SCENARIO;
        }
        menu.mEpisodeListPane->mChildrenList.append(&scenarioText->mPtrLink);

        J2DTextBox *episodeFileNameText = new J2DTextBox(
            ('f' << 24) | scenarioNameID & 0xFFFFFF,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *episodeFileNameTextBuf = new char[100];
            memset(episodeFileNameTextBuf, 0, 100);
            snprintf(episodeFileNameTextBuf, 100, "%s", filename);

            episodeFileNameText->mStrPtr         = episodeFileNameTextBuf;
            episodeFileNameText->mCharSizeX      = 21;
            episodeFileNameText->mCharSizeY      = textHeight;
            episodeFileNameText->mNewlineSize    = textHeight;
            episodeFileNameText->mGradientBottom = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mGradientTop    = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mIsVisible      = false;
        }
        menu.mEpisodeListPane->mChildrenList.append(&episodeFileNameText->mPtrLink);

        EpisodeMenuInfo *episodeInfo  = new EpisodeMenuInfo();
        episodeInfo->mScenarioTextBox = scenarioText;
        episodeInfo->mFilenameTextBox = episodeFileNameText;
        episodeInfo->mNormalStageID   = normalStageID;
        episodeInfo->mScenarioID      = j;
        menu.mEpisodeInfos.insert(menu.mEpisodeInfos.end(), episodeInfo);

        en += 1;
        eny += 1;
    }

    size_t exrow = 0;
    for (s32 j = 0; j < BETTER_SMS_EXAREA_MAX; ++j) {
        const Stage::ExAreaInfo &exinfo = Stage::getExAreaInfos()[j];
        if (exinfo.mShineStageID == -1) {
            continue;
        }

        if (shineStageID != exinfo.mShineStageID) {
            continue;
        }

        char filename[128];
        if (!sceneFilename(filename, 128, j, 0)) {
            continue;
        }

        const TGlobalVector<s32> &exScenarioNameIDs = info->getExScenarioNameIDs();

        s32 exareaNameID = exScenarioNameIDs[exrow];

        J2DTextBox *episodeText = new J2DTextBox(
            ('e' << 24) | j,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *episodeTextBuf = new char[50];
            memset(episodeTextBuf, 0, 50);
            snprintf(episodeTextBuf, 50, "Secret Course %ld", exrow);

            episodeText->mStrPtr         = episodeTextBuf;
            episodeText->mCharSizeX      = 21;
            episodeText->mCharSizeY      = textHeight;
            episodeText->mNewlineSize    = textHeight;
            episodeText->mGradientBottom = TEXT_COLOR_DEFAULT_SCENARIO;
            episodeText->mGradientTop    = TEXT_COLOR_DEFAULT_SCENARIO;
        }
        menu.mEpisodeListPane->mChildrenList.append(&episodeText->mPtrLink);

        J2DTextBox *episodeFileNameText = new J2DTextBox(
            ('f' << 24) | j,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *episodeFileNameTextBuf = new char[100];
            memset(episodeFileNameTextBuf, 0, 100);
            snprintf(episodeFileNameTextBuf, 100, "%s", filename);

            episodeFileNameText->mStrPtr         = episodeFileNameTextBuf;
            episodeFileNameText->mCharSizeX      = 21;
            episodeFileNameText->mCharSizeY      = textHeight;
            episodeFileNameText->mNewlineSize    = textHeight;
            episodeFileNameText->mGradientBottom = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mGradientTop    = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mIsVisible      = false;
        }
        menu.mEpisodeListPane->mChildrenList.append(&episodeFileNameText->mPtrLink);

        EpisodeMenuInfo *episodeInfo  = new EpisodeMenuInfo();
        episodeInfo->mScenarioTextBox = episodeText;
        episodeInfo->mFilenameTextBox = episodeFileNameText;
        episodeInfo->mNormalStageID   = j;
        episodeInfo->mScenarioID      = 0;
        menu.mEpisodeInfos.insert(menu.mEpisodeInfos.end(), episodeInfo);

        eny += 1;
        exrow += 1;
    }
}

void LevelSelectScreen::genEpisodeTextDelfinoPlaza(AreaMenuInfo &menu, u8 normalStageID,
                                                   u8 shineStageID, void *scenarioNameData) {
    auto *areaInfoAry = reinterpret_cast<TNameRefAryT<TScenarioArchiveName> *>(
        gpApplication.mStageArchiveAry->mChildren[1]);

    const Stage::ShineAreaInfo *info = Stage::getShineAreaInfos()[shineStageID];

    size_t rows = 0;
    for (s32 i = 0; i < areaInfoAry->mChildren.size(); ++i) {
        if (!sceneExists(1, i)) {
            continue;
        }
        rows += 1;
    }

    for (s32 i = 0; i < BETTER_SMS_EXAREA_MAX; ++i) {
        const Stage::ExAreaInfo &exinfo = Stage::getExAreaInfos()[i];
        if (exinfo.mShineStageID == -1) {
            continue;
        }

        if (shineStageID != exinfo.mShineStageID) {
            continue;
        }

        if (!sceneExists(i, 0)) {
            continue;
        }
        rows += 1;
    }

    size_t textBaseHeight = 21;
    size_t textHeight     = rows < 10 ? textBaseHeight : (textBaseHeight - (rows - 10));

    s32 en = 0, eny = 0;
    for (s32 i = 0; i < areaInfoAry->mChildren.size(); ++i) {
        char filename[128];
        if (!sceneFilename(filename, 128, 1, i)) {
            continue;
        }

        J2DTextBox *episodeText = new J2DTextBox(
            ('e' << 24) | ('d' << 16) | i,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *episodeTextBuf = new char[100];
            memset(episodeTextBuf, 0, 100);
            snprintf(episodeTextBuf, 100, "Scenario %ld", i);

            episodeText->mStrPtr         = episodeTextBuf;
            episodeText->mCharSizeX      = 21;
            episodeText->mCharSizeY      = textHeight;
            episodeText->mNewlineSize    = textHeight;
            episodeText->mGradientBottom = TEXT_COLOR_DEFAULT_SCENARIO;
            episodeText->mGradientTop    = TEXT_COLOR_DEFAULT_SCENARIO;
        }
        menu.mEpisodeListPane->mChildrenList.append(&episodeText->mPtrLink);

        J2DTextBox *episodeFileNameText = new J2DTextBox(
            ('f' << 24) | ('d' << 16) | i,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *episodeFileNameTextBuf = new char[100];
            memset(episodeFileNameTextBuf, 0, 100);
            snprintf(episodeFileNameTextBuf, 100, "%s", filename);

            episodeFileNameText->mStrPtr         = episodeFileNameTextBuf;
            episodeFileNameText->mCharSizeX      = 21;
            episodeFileNameText->mCharSizeY      = textHeight;
            episodeFileNameText->mNewlineSize    = textHeight;
            episodeFileNameText->mGradientBottom = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mGradientTop    = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mIsVisible      = false;
        }
        menu.mEpisodeListPane->mChildrenList.append(&episodeFileNameText->mPtrLink);

        EpisodeMenuInfo *episodeInfo  = new EpisodeMenuInfo();
        episodeInfo->mScenarioTextBox = episodeText;
        episodeInfo->mFilenameTextBox = episodeFileNameText;
        episodeInfo->mNormalStageID   = normalStageID;
        episodeInfo->mScenarioID      = i;
        menu.mEpisodeInfos.insert(menu.mEpisodeInfos.end(), episodeInfo);

        en += 1;
        eny += 1;
    }

    size_t exrow = 0;
    for (s32 i = 0; i < BETTER_SMS_EXAREA_MAX; ++i) {
        const Stage::ExAreaInfo &exinfo = Stage::getExAreaInfos()[i];
        if (exinfo.mShineStageID == -1) {
            continue;
        }

        if (shineStageID != exinfo.mShineStageID) {
            continue;
        }

        char filename[128];
        if (!sceneFilename(filename, 128, i, 0)) {
            continue;
        }

        const TGlobalVector<s32> &exScenarioNameIDs = info->getExScenarioNameIDs();

        s32 exareaNameID = exScenarioNameIDs[exrow];

        J2DTextBox *episodeText = new J2DTextBox(
            ('e' << 24) | i,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *episodeTextBuf = new char[50];
            memset(episodeTextBuf, 0, 50);
            snprintf(episodeTextBuf, 50, "Secret Course %ld", exrow);

            episodeText->mStrPtr         = episodeTextBuf;
            episodeText->mCharSizeX      = 21;
            episodeText->mCharSizeY      = textHeight;
            episodeText->mNewlineSize    = textHeight;
            episodeText->mGradientBottom = TEXT_COLOR_DEFAULT_SCENARIO;
            episodeText->mGradientTop    = TEXT_COLOR_DEFAULT_SCENARIO;
        }
        menu.mEpisodeListPane->mChildrenList.append(&episodeText->mPtrLink);

        J2DTextBox *episodeFileNameText = new J2DTextBox(
            ('f' << 24) | i,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *episodeFileNameTextBuf = new char[100];
            memset(episodeFileNameTextBuf, 0, 100);
            snprintf(episodeFileNameTextBuf, 100, "%s", filename);

            episodeFileNameText->mStrPtr         = episodeFileNameTextBuf;
            episodeFileNameText->mCharSizeX      = 21;
            episodeFileNameText->mCharSizeY      = textHeight;
            episodeFileNameText->mNewlineSize    = textHeight;
            episodeFileNameText->mGradientBottom = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mGradientTop    = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mIsVisible      = false;
        }
        menu.mEpisodeListPane->mChildrenList.append(&episodeFileNameText->mPtrLink);

        EpisodeMenuInfo *episodeInfo  = new EpisodeMenuInfo();
        episodeInfo->mScenarioTextBox = episodeText;
        episodeInfo->mFilenameTextBox = episodeFileNameText;
        episodeInfo->mNormalStageID   = i;
        episodeInfo->mScenarioID      = 0;
        menu.mEpisodeInfos.insert(menu.mEpisodeInfos.end(), episodeInfo);

        eny += 1;
        exrow += 1;
    }
}

void LevelSelectScreen::genEpisodeTextTest1(AreaMenuInfo &menu) {
    auto *areaInfoAry = reinterpret_cast<TNameRefAryT<TScenarioArchiveName> *>(
        gpApplication.mStageArchiveAry->mChildren[12]);

    size_t rows = 0;
    for (s32 i = 0; i < areaInfoAry->mChildren.size(); ++i) {
        if (!sceneExists(12, i)) {
            continue;
        }
        rows += 1;
    }

    size_t textBaseHeight = 21;
    size_t textHeight     = rows < 10 ? textBaseHeight : (textBaseHeight - (rows - 10));

    s32 en = 0, eny = 0;
    for (s32 i = 0; i < areaInfoAry->mChildren.size(); ++i) {
        char filename[128];
        if (!sceneFilename(filename, 128, 12, i)) {
            continue;
        }

        J2DTextBox *episodeText = new J2DTextBox(
            ('e' << 24) | ('t' << 16) | ('1' << 8) | i,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *episodeTextBuf = new char[100];
            memset(episodeTextBuf, 0, 100);
            snprintf(episodeTextBuf, 100, "Test %ld", 10 + i);

            episodeText->mStrPtr         = episodeTextBuf;
            episodeText->mCharSizeX      = 21;
            episodeText->mCharSizeY      = textHeight;
            episodeText->mNewlineSize    = textHeight;
            episodeText->mGradientBottom = TEXT_COLOR_DEFAULT_SCENARIO;
            episodeText->mGradientTop    = TEXT_COLOR_DEFAULT_SCENARIO;
        }
        menu.mEpisodeListPane->mChildrenList.append(&episodeText->mPtrLink);

        J2DTextBox *episodeFileNameText = new J2DTextBox(
            ('f' << 24) | ('t' << 16) | ('1' << 8) | i,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *episodeFileNameTextBuf = new char[100];
            memset(episodeFileNameTextBuf, 0, 100);
            snprintf(episodeFileNameTextBuf, 100, "%s", filename);

            episodeFileNameText->mStrPtr         = episodeFileNameTextBuf;
            episodeFileNameText->mCharSizeX      = 21;
            episodeFileNameText->mCharSizeY      = textHeight;
            episodeFileNameText->mNewlineSize    = textHeight;
            episodeFileNameText->mGradientBottom = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mGradientTop    = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mIsVisible      = false;
        }
        menu.mEpisodeListPane->mChildrenList.append(&episodeFileNameText->mPtrLink);

        EpisodeMenuInfo *episodeInfo  = new EpisodeMenuInfo();
        episodeInfo->mScenarioTextBox = episodeText;
        episodeInfo->mFilenameTextBox = episodeFileNameText;
        episodeInfo->mNormalStageID   = 12;
        episodeInfo->mScenarioID      = i;
        menu.mEpisodeInfos.insert(menu.mEpisodeInfos.end(), episodeInfo);

        en += 1;
        eny += 1;
    }
}

void LevelSelectScreen::genEpisodeTextTest2(AreaMenuInfo &menu) {
    auto *areaInfoAry = reinterpret_cast<TNameRefAryT<TScenarioArchiveName> *>(
        gpApplication.mStageArchiveAry->mChildren[17]);

    size_t rows = 0;
    for (s32 i = 0; i < areaInfoAry->mChildren.size(); ++i) {
        if (!sceneExists(17, i)) {
            continue;
        }
        rows += 1;
    }

    size_t textBaseHeight = 21;
    size_t textHeight     = rows < 10 ? textBaseHeight : (textBaseHeight - (rows - 10));

    s32 en = 0, eny = 0;
    for (s32 i = 0; i < areaInfoAry->mChildren.size(); ++i) {
        char filename[128];
        if (!sceneFilename(filename, 128, 17, i)) {
            continue;
        }

        J2DTextBox *episodeText = new J2DTextBox(
            ('e' << 24) | ('t' << 16) | ('1' << 8) | i,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *episodeTextBuf = new char[100];
            memset(episodeTextBuf, 0, 100);
            snprintf(episodeTextBuf, 100, "Test %ld", 20 + i);

            episodeText->mStrPtr         = episodeTextBuf;
            episodeText->mCharSizeX      = 21;
            episodeText->mCharSizeY      = textHeight;
            episodeText->mNewlineSize    = textHeight;
            episodeText->mGradientBottom = TEXT_COLOR_DEFAULT_SCENARIO;
            episodeText->mGradientTop    = TEXT_COLOR_DEFAULT_SCENARIO;
        }
        menu.mEpisodeListPane->mChildrenList.append(&episodeText->mPtrLink);

        J2DTextBox *episodeFileNameText = new J2DTextBox(
            ('f' << 24) | ('t' << 16) | ('2' << 8) | i,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *episodeFileNameTextBuf = new char[100];
            memset(episodeFileNameTextBuf, 0, 100);
            snprintf(episodeFileNameTextBuf, 100, "%s", filename);

            episodeFileNameText->mStrPtr         = episodeFileNameTextBuf;
            episodeFileNameText->mCharSizeX      = 21;
            episodeFileNameText->mCharSizeY      = textHeight;
            episodeFileNameText->mNewlineSize    = textHeight;
            episodeFileNameText->mGradientBottom = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mGradientTop    = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mIsVisible      = false;
        }
        menu.mEpisodeListPane->mChildrenList.append(&episodeFileNameText->mPtrLink);

        EpisodeMenuInfo *episodeInfo  = new EpisodeMenuInfo();
        episodeInfo->mScenarioTextBox = episodeText;
        episodeInfo->mFilenameTextBox = episodeFileNameText;
        episodeInfo->mNormalStageID   = 17;
        episodeInfo->mScenarioID      = i;
        menu.mEpisodeInfos.insert(menu.mEpisodeInfos.end(), episodeInfo);

        en += 1;
        eny += 1;
    }
}

void LevelSelectScreen::genEpisodeTextScale(AreaMenuInfo &menu) {
    auto *areaInfoAry = reinterpret_cast<TNameRefAryT<TScenarioArchiveName> *>(
        gpApplication.mStageArchiveAry->mChildren[11]);

    size_t rows = 0;
    for (s32 i = 0; i < areaInfoAry->mChildren.size(); ++i) {
        if (!sceneExists(11, i)) {
            continue;
        }
        rows += 1;
    }

    size_t textBaseHeight = 21;
    size_t textHeight     = rows < 10 ? textBaseHeight : (textBaseHeight - (rows - 10));

    s32 en = 0, eny = 0;
    for (s32 i = 0; i < areaInfoAry->mChildren.size(); ++i) {
        char filename[128];
        if (!sceneFilename(filename, 128, 11, i)) {
            continue;
        }

        J2DTextBox *episodeText = new J2DTextBox(
            ('e' << 24) | ('s' << 16) | i,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *episodeTextBuf = new char[100];
            memset(episodeTextBuf, 0, 100);
            snprintf(episodeTextBuf, 100, "Scale %ld", i);

            episodeText->mStrPtr         = episodeTextBuf;
            episodeText->mCharSizeX      = 21;
            episodeText->mCharSizeY      = textHeight;
            episodeText->mNewlineSize    = textHeight;
            episodeText->mGradientBottom = TEXT_COLOR_DEFAULT_SCENARIO;
            episodeText->mGradientTop    = TEXT_COLOR_DEFAULT_SCENARIO;
        }
        menu.mEpisodeListPane->mChildrenList.append(&episodeText->mPtrLink);

        J2DTextBox *episodeFileNameText = new J2DTextBox(
            ('f' << 24) | ('s' << 16) | i,
            {0, static_cast<int>(110 + ((textHeight + 2) * eny)), 600,
             static_cast<int>(158 + ((textHeight + 2) * eny))},
            gpSystemFont->mFont, "", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        {
            char *episodeFileNameTextBuf = new char[100];
            memset(episodeFileNameTextBuf, 0, 100);
            snprintf(episodeFileNameTextBuf, 100, "%s", filename);

            episodeFileNameText->mStrPtr         = episodeFileNameTextBuf;
            episodeFileNameText->mCharSizeX      = 21;
            episodeFileNameText->mCharSizeY      = textHeight;
            episodeFileNameText->mNewlineSize    = textHeight;
            episodeFileNameText->mGradientBottom = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mGradientTop    = TEXT_COLOR_DEFAULT_FILENAME;
            episodeFileNameText->mIsVisible      = false;
        }
        menu.mEpisodeListPane->mChildrenList.append(&episodeFileNameText->mPtrLink);

        EpisodeMenuInfo *episodeInfo  = new EpisodeMenuInfo();
        episodeInfo->mScenarioTextBox = episodeText;
        episodeInfo->mFilenameTextBox = episodeFileNameText;
        episodeInfo->mNormalStageID   = 11;
        episodeInfo->mScenarioID      = i;
        menu.mEpisodeInfos.insert(menu.mEpisodeInfos.end(), episodeInfo);

        en += 1;
        eny += 1;
    }
}

J2DPane *LevelSelectScreen::findOrCreateAreaPane(u8 normalStageID, u8 shineStageID, int width,
                                                 int height, bool *created) {
    for (AreaMenuInfo *menu : mAreaInfos) {
        if (menu->mStageID == shineStageID) {
            if (created) {
                *created = false;
            }
            return menu->mEpisodeListPane;
        }
    }

    if (created) {
        *created = true;
    }

    J2DPane *areaPane    = new J2DPane(19, ('s' << 24) | normalStageID, {0, 0, width, height});
    areaPane->mIsVisible = false;
    return areaPane;
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

static JKRMemArchive *s_title_archive = nullptr;

void LevelSelectDirector::initialize() {
    void *archive   = SMSLoadArchive("/data/title.arc", nullptr, 0, nullptr);
    s_title_archive = new JKRMemArchive();
    s_title_archive->mountFixed(archive, JKRMemBreakFlag::UNK_0);

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

    {
        auto *stageDisp = new JDrama::TDStageDisp("<DStageDisp>", {0});

        auto *efbCtrl = stageDisp->getEfbCtrlDisp();
        efbCtrl->setSrcRect(screenRect);

        rootObjGroup->mViewObjList.insert(rootObjGroup->mViewObjList.end(), stageDisp);
        stageObjGroup->mViewObjList.insert(stageObjGroup->mViewObjList.end(), stageDisp);
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
        const ResTIMG *bg_timg = reinterpret_cast<const ResTIMG *>(
            JKRFileLoader::getGlbResource("/title/timg/title_test.bti"));
        if (bg_timg) {
            JUTTexture *bg_texture      = new JUTTexture();
            bg_texture->mTexObj2.val[2] = 0;
            bg_texture->storeTIMG(bg_timg);
            bg_texture->_50 = false;

            J2DPicture *background =
                new J2DPicture('BG_0', {0, 0, screenOrthoWidth, screenRenderHeight});
            background->insert(bg_texture, 0, 1.0f);

            // Darken bg
            background->mColorOverlay    = {0, 0, 0, 128};
            background->mVertexColors[0] = {100, 100, 100, 128};
            background->mVertexColors[1] = {100, 100, 100, 128};
            background->mVertexColors[2] = {100, 100, 100, 128};
            background->mVertexColors[3] = {100, 100, 100, 128};

            mSelectScreen->mScreen->mChildrenList.append(&background->mPtrLink);
        }

        J2DTextBox *label = new J2DTextBox(
            'logo', {0, screenRenderHeight - 110, 600, screenRenderHeight}, gpSystemFont->mFont,
            "Level Select", J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
        label->mCharSizeX   = 24;
        label->mCharSizeY   = 24;
        label->mNewlineSize = 24;
        mSelectScreen->mScreen->mChildrenList.append(&label->mPtrLink);

        J2DTextBox *exitLabel = new J2DTextBox(
            'exit',
            {static_cast<int>(20 - BetterSMS::getScreenRatioAdjustX()), screenRenderHeight - 110,
             static_cast<int>(100 - BetterSMS::getScreenRatioAdjustX()), screenRenderHeight},
            gpSystemFont->mFont, "# Exit", J2DTextBoxHBinding::Left, J2DTextBoxVBinding::Center);
        mSelectScreen->mScreen->mChildrenList.append(&exitLabel->mPtrLink);
    }

    BetterSMS::Stage::NormalAreaInfo *normalAreaInfos = BetterSMS::Stage::getNormalAreaInfos();
    BetterSMS::Stage::ExAreaInfo *exAreaInfos   = BetterSMS::Stage::getExAreaInfos();

    bool visited_map[BETTER_SMS_AREA_MAX];
    memset(visited_map, 0, sizeof(visited_map));

    size_t areaCount = 3;  // Account for test maps and scale map
    for (s32 i = 0; i < BETTER_SMS_AREA_MAX; ++i) {
        if (normalAreaInfos[i].mShineStageID == -1) {
            continue;
        }
        if (visited_map[normalAreaInfos[i].mShineStageID] == true) {
            continue;
        }
        if (Stage::isExStage(i, 0)) {
            continue;
        }
        visited_map[normalAreaInfos[i].mShineStageID] = true;
        areaCount += 1;
    }

    size_t rowsPerColumn = 14;
    size_t columns       = (areaCount / rowsPerColumn) + 1;

    mSelectScreen->mColumnSize  = rowsPerColumn;
    mSelectScreen->mColumnCount = columns;

    s32 flatRow = 0;
    for (s32 i = 0; i < BETTER_SMS_AREA_MAX; ++i) {
        if (normalAreaInfos[i].mShineStageID == -1 || visited_map[normalAreaInfos[i].mShineStageID] == false) {
            continue;
        }
        switch (i) {
        case 11:
            if (mSelectScreen->genAreaTextScale(flatRow)) {
                flatRow += 1;
            }
            break;
        case 12:
            if (mSelectScreen->genAreaTextTest1(flatRow)) {
                flatRow += 1;
            }
            break;
        case 17:
            if (mSelectScreen->genAreaTextTest2(flatRow)) {
                flatRow += 1;
            }
            break;
        default:
            if (mSelectScreen->genAreaText(flatRow, i, normalAreaInfos[i].mShineStageID, stageNameData,
                                           scenarioNameData)) {
                flatRow += 1;
            }
            break;
        }
    }
}

s32 LevelSelectDirector::direct() {
    s32 ret = 1;

    int *joinBuf[2];

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

        if (!TFlagManager::smInstance->getBool(0x30007)) {
            TFlagManager::smInstance->setBool(true, 0x30007);
            gpMSound->loadWave(MS_WAVE_DEFAULT);
        }

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
                EpisodeMenuInfo *info = mSelectScreen->mAreaInfos[mSelectScreen->mSelectedAreaID]
                                            ->mEpisodeInfos[mSelectScreen->mSelectedEpisodeID];
                gpApplication.mNextScene.mAreaID    = info->mNormalStageID;
                gpApplication.mNextScene.mEpisodeID = info->mScenarioID;
                // Reset coins
                TFlagManager::smInstance->setFlag(0x40002, 0);
                TFlagManager::smInstance->setFlag(0x40003, info->mScenarioID);
            }

            if ((mController->mButtons.mInput & TMarioGamePad::X)) {
                TFlagManager::smInstance->firstStart();
                for (u32 shine = 0; shine < BetterSMS::Game::getMaxShines(); ++shine) {
                    TFlagManager::smInstance->setShineFlag(shine);
                }
                for (u32 bluec = 0x10366; bluec < 0x103B4; ++bluec) {
                    TFlagManager::smInstance->setBool(true, bluec);
                }
                TFlagManager::smInstance->saveSuccess();
            }

            mState = State::EXIT;
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

    Loading::setLoading(true);

    if (!gpMSound->checkWaveOnAram((MS_SCENE_WAVE)517)) {
        return 1;
    }

    if (fader->mFadeStatus == TSMSFader::FADE_OFF) {
        gpApplication.mFader->startFadeoutT(0.3f);
        MSBgm::stopBGM(BGM_MARE_SEA, 10);
    }
    return fader->mFadeStatus == TSMSFader::FADE_ON ? 5 : 1;
}