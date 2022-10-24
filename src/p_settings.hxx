#pragma once

#include <Dolphin/types.h>
#include <Dolphin/printf.h>
#include <Dolphin/string.h>

#include <JSystem/JDrama/JDRDirector.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>

#include <SMS/manager/FlagManager.hxx>
#include <SMS/GC2D/SelectGrad.hxx>
#include <SMS/MarioUtil/DrawUtil.hxx>
#include <SMS/MarioUtil/gd-reinit-gx.hxx>
#include <SMS/System/Resolution.hxx>

#include "libs/anim2d.hxx"
#include "libs/container.hxx"
#include "icons.hxx"
#include "settings.hxx"
#include "module.hxx"

using namespace BetterSMS;

static const u8 *sLoadingIconTIMGs[] = {
    gShineSpriteIconFrame1,  gShineSpriteIconFrame2,  gShineSpriteIconFrame3,
    gShineSpriteIconFrame4,  gShineSpriteIconFrame5,  gShineSpriteIconFrame6,
    gShineSpriteIconFrame7,  gShineSpriteIconFrame8,  gShineSpriteIconFrame9,
    gShineSpriteIconFrame10, gShineSpriteIconFrame11, gShineSpriteIconFrame12,
    gShineSpriteIconFrame13, gShineSpriteIconFrame14, gShineSpriteIconFrame15,
    gShineSpriteIconFrame16
};

void getSettingsGroups(TDictS<Settings::SettingsGroup *>::ItemList &out);

inline int getTextWidth(J2DTextBox *textbox) {
    const size_t textLength = strlen(textbox->mStrPtr);

    size_t textWidth = 0;
    for (int i = 0; i < textLength; ++i) {
        JUTFont::TWidth width;
        textbox->mFont->getWidthEntry(textbox->mStrPtr[i], &width);
        textWidth += width.mWidth;
    }

    return textWidth + (Max(textLength - 1, 0) * textbox->mCharSpacing);
}

inline void centerTextBoxX(J2DTextBox* textbox, int width) {
    int textWidth      = getTextWidth(textbox);
    textbox->mRect.mX1 = ((width >> 1) - (getScreenRatioAdjustX() / 2)) -
                         (getTextWidth(textbox) >> 1) - textbox->mCharSizeX - 4;
    textbox->mRect.mX2 = textbox->mRect.mX1 + textWidth + (strlen(textbox->mStrPtr) * textbox->mCharSizeX);
}

struct SettingInfo {
    J2DTextBox *mSettingTextBox;
    J2DTextBox *mSettingTextBoxBack;
    Settings::SingleSetting *mSettingData;
};

struct GroupInfo {
    J2DPane *mGroupPane;
    Settings::SettingsGroup *mSettingGroup;
    JGadget::TList<SettingInfo *> mSettingInfos;
};

class SettingsScreen : public JDrama::TViewObj {
public:
    friend class SettingsDirector;

    SettingsScreen(TMarioGamePad *controller)
        : TViewObj("<SettingsScreen>"),
          mScreen(nullptr), mController(controller), mShineIcon(nullptr), mCurrentTextBox(nullptr), mGroupID(0),
          mSettingID(0), mGroups() {
        mShineAnimator = SimpleTexAnimator(sLoadingIconTIMGs, 16);
    }

    ~SettingsScreen() override {}

    void perform(u32 flags, JDrama::TGraphics *graphics) override {
        if ((flags & 0x1)) {
            processInput();
        }

        if ((flags & 0x3)) {
            const u32 groupMagic = ('p' << 24) | mGroupID;
            J2DPane *groupPane   = mScreen->search(groupMagic);
            if (!groupPane)
                return;

            if (groupPane->mChildrenList.mSize == 0) {
                //mShineIcon->mIsVisible = false;
                mCurrentTextBox        = nullptr;
                return;
            }

            //mShineIcon->mIsVisible = true;

            const u32 settingMagic = ('s' << 24) | mSettingID;
            J2DPane *settingPane   = groupPane->search(settingMagic);
            if (!settingPane)
                return;

            const u32 settingBackMagic = ('b' << 24) | mSettingID;
            J2DPane *settingBackPane   = groupPane->search(settingBackMagic);
            if (!settingBackPane)
                return;

            if (mCurrentTextBox != settingPane) {
                //mShineIcon->move(settingPane->mRect.mX1 - 32, settingPane->mRect.mY1);
                if (mCurrentTextBox) {
                    mCurrentTextBox->mGradientTop    = {255, 255, 255, 255};
                    mCurrentTextBox->mGradientBottom = {255, 255, 255, 255};
                }
                mCurrentTextBox                  = reinterpret_cast<J2DTextBox *>(settingPane);
                mCurrentTextBox->mGradientTop    = {180, 230, 10, 255};
                mCurrentTextBox->mGradientBottom = {240, 170, 10, 255};
            }

            /*mShineAnimator.process(mShineIcon);*/
        }

        if ((flags & 0x8)) {
            ReInitializeGX();
            SMS_DrawInit();

            J2DOrthoGraph ortho(0, 0, BetterSMS::getScreenOrthoWidth(), SMSGetTitleRenderHeight());
            ortho.setup2D();

            mScreen->draw(0, 0, &ortho);
        }
    }

private:
    void processInput() {
        {
            auto currentID = mSettingID;

            if ((mController->mFrameMeaning & 0x4)) {
                for (int i = mSettingID + 1; i < mCurrentGroupInfo->mSettingInfos.size(); ++i) {
                    auto *settingInfo = getSettingInfo(i);
                    if (settingInfo->mSettingData->isUserEditable()) {
                        mCurrentSettingInfo = settingInfo;
                        mSettingID          = i;
                        break;
                    }
                }
            }
            if ((mController->mFrameMeaning & 0x2)) {
                for (int i = mSettingID - 1; i >= 0; --i) {
                    auto *settingInfo = getSettingInfo(i);
                    if (settingInfo->mSettingData->isUserEditable()) {
                        mCurrentSettingInfo = settingInfo;
                        mSettingID          = i;
                        break;
                    }
                }
            }
        }

        if (mController->mFrameMeaning & 0x10) {
            mCurrentSettingInfo->mSettingData->nextValue();
            {
                char valueTextBuf[40];
                mCurrentSettingInfo->mSettingData->getValueStr(valueTextBuf);

                snprintf(mCurrentSettingInfo->mSettingTextBox->mStrPtr, 100, "%s: %s",
                         mCurrentSettingInfo->mSettingData->getName(), valueTextBuf);
            }
            centerTextBoxX(mCurrentSettingInfo->mSettingTextBox, BetterSMS::getScreenRenderWidth());
            mCurrentSettingInfo->mSettingTextBox->mRect.mY1 = 110 + (28 * mSettingID);
            mCurrentSettingInfo->mSettingTextBox->mRect.mY2 = 158 + (28 * mSettingID);

            centerTextBoxX(mCurrentSettingInfo->mSettingTextBoxBack, BetterSMS::getScreenRenderWidth());
            mCurrentSettingInfo->mSettingTextBoxBack->mRect.mX1 += 2;
            mCurrentSettingInfo->mSettingTextBoxBack->mRect.mX2 += 2;
            mCurrentSettingInfo->mSettingTextBoxBack->mRect.mY1 = 110 + (28 * mSettingID) + 2;
            mCurrentSettingInfo->mSettingTextBoxBack->mRect.mY2 = 158 + (28 * mSettingID) + 2;
        }

        if (mController->mFrameMeaning & 0x8) {
            mCurrentSettingInfo->mSettingData->prevValue();
            {
                char valueTextBuf[40];
                mCurrentSettingInfo->mSettingData->getValueStr(valueTextBuf);

                snprintf(mCurrentSettingInfo->mSettingTextBox->mStrPtr, 100, "%s: %s",
                         mCurrentSettingInfo->mSettingData->getName(), valueTextBuf);
            }
            centerTextBoxX(mCurrentSettingInfo->mSettingTextBox, BetterSMS::getScreenRenderWidth());
            mCurrentSettingInfo->mSettingTextBox->mRect.mY1 = 110 + (28 * mSettingID);
            mCurrentSettingInfo->mSettingTextBox->mRect.mY2 = 158 + (28 * mSettingID);

            centerTextBoxX(mCurrentSettingInfo->mSettingTextBoxBack, BetterSMS::getScreenRenderWidth());
            mCurrentSettingInfo->mSettingTextBoxBack->mRect.mX1 += 2;
            mCurrentSettingInfo->mSettingTextBoxBack->mRect.mX2 += 2;
            mCurrentSettingInfo->mSettingTextBoxBack->mRect.mY1 = 110 + (28 * mSettingID) + 2;
            mCurrentSettingInfo->mSettingTextBoxBack->mRect.mY2 = 158 + (28 * mSettingID) + 2;
        }

        {
            auto oldID = mGroupID;

            if ((mController->mButtons.mFrameInput & TMarioGamePad::R)) {
                mGroupID += 1;
            }
            if ((mController->mButtons.mFrameInput & TMarioGamePad::L)) {
                mGroupID -= 1;
            }
            mGroupID = Clamp(mGroupID, 0, mGroups.size() - 1);

            if (oldID != mGroupID) {
                mCurrentGroupInfo->mGroupPane->mIsVisible = false;
                mCurrentGroupInfo = getGroupInfo(mGroupID);
                mCurrentGroupInfo->mGroupPane->mIsVisible = true;
                mCurrentSettingInfo = getSettingInfo(0);
                mSettingID          = 0;
            }
        }
    }

    GroupInfo *getGroupInfo(u32 index) {
        auto it = mGroups.begin();
        for (int i = 0; i < mGroups.size(); ++i, ++it) {
            if (i == index)
                return *it;
        }
        return nullptr;
    }

    SettingInfo *getSettingInfo(u32 index) {
        auto it = mCurrentGroupInfo->mSettingInfos.begin();
        for (int i = 0; i < mCurrentGroupInfo->mSettingInfos.size(); ++i, ++it) {
            if (i == index)
                return *it;
        }
        return nullptr;
    }

    s32 mGroupID;
    s32 mSettingID;
    TMarioGamePad *mController;
    J2DScreen *mScreen;
    J2DPicture *mShineIcon;
    J2DTextBox *mCurrentTextBox;
    GroupInfo *mCurrentGroupInfo;
    SettingInfo *mCurrentSettingInfo;
    SimpleTexAnimator mShineAnimator;
    JGadget::TList<GroupInfo *> mGroups;
};

class SettingsDirector : public JDrama::TDirector {
    enum class State { INIT, GROUP, SETTING, EXIT };

public:
    SettingsDirector()
        : TDirector(), mState(State::INIT), mDisplay(nullptr),
          mController(nullptr) {}
    ~SettingsDirector() override;

    s32 direct() override;
    void setup(JDrama::TDisplay *, TMarioGamePad *);

private:
    void initialize();
    void initializeLayout();
    void saveSettings();

    s32 exit();

    static void *setupThreadFunc(void *);

private:
    State mState;
    JDrama::TDisplay *mDisplay;
    TMarioGamePad *mController;
    SettingsScreen *mSettingScreen;
    TSelectGrad *mGradBG;
};

class RumbleSetting final : public Settings::SwitchSetting {
public:
    RumbleSetting(const char *name) : SwitchSetting(name, &RumbleSetting::sRumbleFlag) {
        mValueChangedCB = RumbleSetting::valueChanged;
    }

private:
    static void valueChanged(void *old, void *cur, ValueKind kind) {
        auto flag = *reinterpret_cast<bool *>(cur);
        TFlagManager::smInstance->setBool(!flag, 0x70000);
        TFlagManager::smInstance->setBool(flag, 0x90000);
    }

    static bool sRumbleFlag;
};

class SubtitleSetting final : public Settings::SwitchSetting {
public:
    SubtitleSetting(const char *name) : SwitchSetting(name, &SubtitleSetting::sSubtitleFlag) {
        mValueChangedCB = SubtitleSetting::valueChanged;
    }

private:
    static void valueChanged(void *old, void *cur, ValueKind kind) {
        auto flag = *reinterpret_cast<bool *>(cur);
        TFlagManager::smInstance->setBool(!flag, 0x70002);
        TFlagManager::smInstance->setBool(flag, 0x90001);
    }

    static bool sSubtitleFlag;
};

class SoundSetting final : public Settings::IntSetting {
public:
    enum Kind { MONO, STEREO, SURROUND };

    SoundSetting(const char *name) : IntSetting(name, &SoundSetting::sSoundValue) {
        mValueRange = {0, 2, 1};
        mValueChangedCB = SoundSetting::valueChanged;
    }
    ~SoundSetting() override {}

    void getValueStr(char *dst) const override {
        switch (getInt()) {
        case Kind::MONO: 
            strncpy(dst, "MONO", 5);
            break;
        case Kind::STEREO:
            strncpy(dst, "STEREO", 7);
            break;
        case Kind::SURROUND:
            strncpy(dst, "SURROUND", 9);
            break;
        default:
            strncpy(dst, "UNKNOWN", 8);
        }
    }

private:
    static void valueChanged(void *old, void *cur, ValueKind kind) {
        auto soundMode = *reinterpret_cast<int *>(cur);
        TFlagManager::smInstance->setFlag(0xA0000, soundMode);
        TFlagManager::smInstance->setBool(soundMode == Kind::SURROUND, 0x70001);
    }

    static int sSoundValue;
};

class AspectRatioSetting final : public Settings::IntSetting {
public:
    enum Kind { FULL, WIDE, ULTRAWIDE };

    AspectRatioSetting(const char *name) : IntSetting(name, &AspectRatioSetting::sAspectRatioValue) {
        mValueRange     = {0, 2, 1};
    }
    ~AspectRatioSetting() override {}

    void getValueStr(char *dst) const override {
        switch (getInt()) {
        default:
        case Kind::FULL:
            strncpy(dst, "4:3", 4);
            break;
        case Kind::WIDE:
            strncpy(dst, "16:9", 5);
            break;
        case Kind::ULTRAWIDE:
            strncpy(dst, "21:9", 5);
            break;
        }
    }

private:
    static int sAspectRatioValue;
};

class FPSSetting final : public Settings::IntSetting {
public:
    enum Kind { FPS_30, FPS_60, FPS_120 };

    FPSSetting(const char *name) : IntSetting(name, &FPSSetting::sFPSValue) {
        mValueRange = {0, 1, 1};
    }
    ~FPSSetting() override {}

    void getValueStr(char *dst) const override {
        switch (getInt()) {
        default:
        case Kind::FPS_30:
            strncpy(dst, "30 FPS", 7);
            break;
        case Kind::FPS_60:
            strncpy(dst, "60 FPS", 7);
            break;
        case Kind::FPS_120:
            strncpy(dst, "120 FPS", 8);
            break;
        }
    }

private:
    static int sFPSValue;
};