#include <Dolphin/types.h>
#include <JSystem/JUtility/JUTColor.hxx>
#include <JSystem/JUtility/JUTRect.hxx>
#include <JSystem/J2D/J2DPicture.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <JSystem/JUtility/JUTTexture.hxx>
#include <SMS/screen/SMSFader.hxx>
#include <SMS/game/Application.hxx>

#include "settings.hxx"
#include "globals.hxx"
#include "module.hxx"

#include "p_settings.hxx"


JGadget::TList<SettingsGroup *> BetterSMS::Settings::getGroups() { return {}; }
SettingsGroup *BetterSMS::Settings::getGroup(const char *groupName) { return nullptr; }

extern const ResTIMG *sShineSpriteIconFrame1;
extern const ResTIMG *sShineSpriteIconFrame1;
extern const ResTIMG *sShineSpriteIconFrame2;
extern const ResTIMG *sShineSpriteIconFrame3;
extern const ResTIMG *sShineSpriteIconFrame4;
extern const ResTIMG *sShineSpriteIconFrame5;
extern const ResTIMG *sShineSpriteIconFrame6;
extern const ResTIMG *sShineSpriteIconFrame7;
extern const ResTIMG *sShineSpriteIconFrame8;
extern const ResTIMG *sShineSpriteIconFrame9;
extern const ResTIMG *sShineSpriteIconFrame10;
extern const ResTIMG *sShineSpriteIconFrame11;
extern const ResTIMG *sShineSpriteIconFrame12;
extern const ResTIMG *sShineSpriteIconFrame13;
extern const ResTIMG *sShineSpriteIconFrame14;
extern const ResTIMG *sShineSpriteIconFrame15;


s32 SettingsDirector::direct() {
    s32 ret = 0;

    switch (mState) {
    case State::INIT: {
        initialize();
        break;
    }
    case State::GROUP: {
        processInput();
        processAnimations();
        break;
    }
    case State::SETTING: {
        processInput();
        processAnimations();
        break;
    }
    case State::EXIT: {
        ret = exit();
        break;
    }
    }
    draw();
    return ret;
}

void SettingsDirector::draw() {
    J2DOrthoGraph ortho(0, 0, BetterSMS::getScreenWidth(), 448);
    mScreen.draw(0, 0, &ortho);
}

s32 SettingsDirector::exit() {
    TSMSFader *fader = gpApplication.mFader;
    if (fader->mFadeStatus == TSMSFader::FADE_OFF)
        gpApplication.mFader->startFadeoutT(0.3f);
    return fader->mFadeStatus == TSMSFader::FADE_ON;
}

void SettingsDirector::initialize() {
    TSMSFader *fader = gpApplication.mFader;

    if (!mHeap) {
        mGroupID   = 0;
        mSettingID = 0;

        mHeap = JKRSolidHeap::create(0x200000, JKRHeap::sRootHeap, false);
        mHeap->becomeCurrentHeap();

        mGroups = BetterSMS::Settings::getGroups();
        initializeLayout();

        fader->startFadeinT(0.3f);
    }

    if (fader->mFadeStatus == TSMSFader::FADE_OFF)
        mState = State::GROUP;
}

void SettingsDirector::initializeLayout() {
    mGroupID   = 0;
    mSettingID = 0;

    int i = 0;
    for (auto &group : mGroups) {
        J2DPane *pane = new J2DPane(19, ('p' << 24) | i, {320 * i, 0, 640, 480});
        {
            J2DTextBox *label = new J2DTextBox(
                ('t' << 24) | i, {0, 40, 640, 32}, gpSystemFont->mFont, group->getName(),
                               J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
            pane->mChildrenList.append(&label->mPtrLink);
        }

        size_t n = 0;
        for (auto &setting : group->getSettings()) {
            {
                JUTTexture *texture      = new JUTTexture();
                texture->mTexObj2.val[2] = 0;
                texture->storeTIMG(sShineSpriteIconFrame1);
                texture->_50 = false;

                J2DPicture *settingSprite = new J2DPicture(('i' << 24) | n, {40, 120, 16, 16});
                settingSprite->insert(texture, 0, 1.0f);

                pane->mChildrenList.append(&settingSprite->mPtrLink);
            }

            {
                J2DTextBox *settingText = new J2DTextBox(
                    ('s' << 24) | n, {80, 120, 560, 16}, gpSystemFont->mFont, group->getName(),
                                   J2DTextBoxHBinding::Center, J2DTextBoxVBinding::Center);
                settingText->mCharSizeX = 20;
                settingText->mCharSizeY = 20;
                settingText->mNewlineSize = 20;
                pane->mChildrenList.append(&settingText->mPtrLink);
            }
            ++n;
        }
        ++i;
    }
}

void SettingsDirector::processAnimations() {}
void SettingsDirector::processInput() {}