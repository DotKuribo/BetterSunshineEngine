#pragma once

#include <JDrama/JDRCamera.hxx>
#include <JDrama/JDRDStageGroup.hxx>
#include <JDrama/JDRDirector.hxx>
#include <JDrama/JDRDisplay.hxx>
#include <JDrama/JDRScreen.hxx>
#include <JGadget/Vector.hxx>
#include <SMS/MarioUtil/DrawUtil.hxx>
#include <SMS/MarioUtil/gd-reinit-gx.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/System/Resolution.hxx>

#include <J2D/J2DOrthoGraph.hxx>

#include "module.hxx"
#include "p_area.hxx"
#include <GC2D/SelectGrad.hxx>

using namespace BetterSMS;

struct EpisodeMenuInfo {
    J2DTextBox *mScenarioTextBox;
    J2DTextBox *mFilenameTextBox;
    s32 mNormalStageID;
    s32 mScenarioID;
};

struct AreaMenuInfo {
    J2DPane *mEpisodeListPane;
    J2DTextBox *mTextBox;
    JGadget::TVector<EpisodeMenuInfo *> mEpisodeInfos;
    s32 mStageID;
};

class LevelSelectScreen : public JDrama::TViewObj {

public:
    friend class LevelSelectDirector;

    LevelSelectScreen(TMarioGamePad *controller)
        : TViewObj("<LevelSelectScreen>"), mScreen(nullptr), mController(controller),
          mScrollAreaID(0), mScrollEpisodeID(0), mSelectedAreaID(-1), mSelectedEpisodeID(-1),
          mAreaInfos(), mShouldExit(false) {}

    ~LevelSelectScreen() override {}

    void perform(u32, JDrama::TGraphics *) override;

    AreaMenuInfo *getAreaInfo(u32 index);
    EpisodeMenuInfo *getEpisodeInfo(u32 index);

protected:
    void processInput();
    bool genAreaText(s32, const Stage::AreaInfo &, void *, void *);
    bool genAreaTextTest1(s32);
    bool genAreaTextTest2(s32);
    bool genAreaTextScale(s32);
    void genEpisodeText(AreaMenuInfo &, const Stage::AreaInfo &, void *);
    void genEpisodeTextDelfinoPlaza(AreaMenuInfo &, const Stage::AreaInfo &, void *);
    void genEpisodeTextTest1(AreaMenuInfo &);
    void genEpisodeTextTest2(AreaMenuInfo &);
    void genEpisodeTextScale(AreaMenuInfo &);
    J2DPane *findOrCreateAreaPane(const Stage::AreaInfo &, int width, int height, bool *created);

private:
    bool mShouldExit;
    s32 mColumnSize;
    s32 mColumnCount;
    s32 mScrollAreaID;
    s32 mScrollEpisodeID;
    s32 mSelectedAreaID;
    s32 mSelectedEpisodeID;
    TMarioGamePad *mController;
    J2DScreen *mScreen;
    JGadget::TVector<AreaMenuInfo *> mAreaInfos;
};

class LevelSelectDirector : public JDrama::TDirector {
    enum class State { INIT, CONTROL, EXIT };

public:
    LevelSelectDirector()           = default;
    ~LevelSelectDirector() override = default;

    void setup(JDrama::TDisplay *, TMarioGamePad *);

    s32 direct() override;

protected:
    static void *setupThreadFunc(void *);

    s32 exit();
    void initialize();
    void initializeDramaHierarchy();
    void initializeLevelsLayout();

private:
    State mState;
    JDrama::TDisplay *mDisplay;
    TMarioGamePad *mController;
    LevelSelectScreen *mSelectScreen;
};