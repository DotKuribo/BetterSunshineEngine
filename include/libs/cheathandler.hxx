#pragma once

#include <SMS/Player/Mario.hxx>

class TCheatHandler {
public:
    TCheatHandler()
        : mInputList(nullptr), mSuccessCallback(nullptr), mFailureCallback(nullptr),
          mGamePad(nullptr), mActive(false), mInputIndex(0) {}

    TCheatHandler(u16 *inputList, TMarioGamePad *gamepad, void (*successCallback)(TCheatHandler *),
                  void (*failureCallback)(TCheatHandler *))
        : mInputList(inputList), mSuccessCallback(successCallback),
          mFailureCallback(failureCallback), mGamePad(gamepad), mActive(false), mInputIndex(0) {}

    void setInputList(u16 *inputList) { mInputList = inputList; }
    void setSuccessCallBack(void (*callback)(TCheatHandler *)) { mSuccessCallback = callback; }
    void setFailureCallBack(void (*callback)(TCheatHandler *)) { mFailureCallback = callback; }
    void setGamePad(TMarioGamePad *gamePad) { mGamePad = gamePad; }

    constexpr bool isActive() const { return (mActive && mInputIndex < 0); }
    constexpr bool isDead() const { return !(mActive && mInputIndex < 0); }
    constexpr bool isInitialized() const { return (mGamePad != nullptr); }

    void advanceInput() {
        if (!mGamePad || !mInputList)
            return;
        else if (mGamePad->mButtons.mFrameInput == 0 || mInputIndex < 0)
            return;

        if (mGamePad->mButtons.mFrameInput != mInputList[static_cast<u8>(mInputIndex)]) {
            fail();
            return;
        }

        if (mInputList[static_cast<u8>(mInputIndex) + 1] == 0) {
            succeed();
            return;
        }

        mInputIndex += 1;
    }

    void succeed() {
        mInputIndex = -1;
        mActive     = true;

        if (mSuccessCallback)
            mSuccessCallback(this);
    }

    void fail() {
        mInputIndex = -1;
        mActive     = false;

        if (mFailureCallback)
            mFailureCallback(this);
    }

    void reset() {
        mInputIndex = 0;
        mActive     = false;
    }

private:
    void (*mSuccessCallback)(TCheatHandler *);
    void (*mFailureCallback)(TCheatHandler *);

    u16 *mInputList;
    TMarioGamePad *mGamePad;
    s8 mInputIndex;
    bool mActive;
};
