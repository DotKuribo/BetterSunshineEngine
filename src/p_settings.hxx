#pragma once

#include <Dolphin/types.h>
#include <JSystem/JDrama/JDRDirector.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>

#include <SMS/GC2D/SelectGrad.hxx>
#include <SMS/MarioUtil/DrawUtil.hxx>
#include <SMS/MarioUtil/gd-reinit-gx.hxx>

#include "settings.hxx"

using namespace BetterSMS;

class SettingsScreen : public JDrama::TViewObj {
public:
    SettingsScreen()
        : TViewObj("<SettingsScreen>"), mScreen(), mGroupPanes(), mGroupID(0), mSettingID(0) {}

    ~SettingsScreen() override {}

    void perform(u32 flags, JDrama::TGraphics *graphics) override {
        if ((flags & 0x8)) {
            ReInitializeGX();
            SMS_DrawInit();

            J2DOrthoGraph ortho(0, 0, BetterSMS::getScreenWidth(), SMSGetTitleRenderHeight());
            ortho.setup2D();

            mScreen.draw(0, 0, &ortho);
        }
    }

    s32 mGroupID;
    s32 mSettingID;
    J2DScreen mScreen;
    JGadget::TList<J2DPane *> mGroupPanes;
};

class SettingsDirector : public JDrama::TDirector {
    enum class State { INIT, GROUP, SETTING, EXIT };

public:
    SettingsDirector()
        : TDirector(), mState(State::INIT), mDisplay(nullptr),
          mController(nullptr), mGroupID(0), mSettingID(0) {}
    ~SettingsDirector() override;

    s32 direct() override;
    void setup(JDrama::TDisplay *, TMarioGamePad *);

private:
    void initialize();
    void initializeLayout();

    void processInput();
    void processAnimations();

    void draw();
    s32 exit();

    static void *setupThreadFunc(void *);

private:
    State mState;
    JDrama::TDisplay *mDisplay;
    TMarioGamePad *mController;
    SettingsScreen *mScreen;
    TSelectGrad *mGradBG;
    s32 mGroupID;
    s32 mSettingID;
};

class SoundSetting final : Settings::SingleSetting {
public:
    enum Kind { MONO, STEREO, SURROUND };

    SoundSetting(const char *name) : SingleSetting(name, &SoundSetting::sSoundValue) { mValueRange = {0, 2, 1}; }
    ~SoundSetting() override {}

    ValueKind getKind() const override { return ValueKind::INT; }
    void getValueStr(char *dst) const override {
        switch (getInt()) {
        case Kind::MONO:
            strncpy(dst, "MONO", 4);
            break;
        case Kind::STEREO:
            strncpy(dst, "STEREO", 6);
            break;
        case Kind::SURROUND:
            strncpy(dst, "SURROUND", 8);
            break;
        default:
            strncpy(dst, "UNKNOWN", 7);
        }
    }
    void setValue(const void *val) const override {
        *reinterpret_cast<int *>(mValuePtr) = *reinterpret_cast<const int *>(val);
    }

    static int sSoundValue;
};