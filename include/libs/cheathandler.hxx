#pragma once

#include <SMS/actor/Mario.hxx>

class TCheatHandler {
public:
    TCheatHandler();
    TCheatHandler(u16 *inputList, TMarioGamePad *gamepad, void (*successCallback)(TCheatHandler *),
                  void (*failureCallback)(TCheatHandler *));

    void setInputList(u16 *inputList) { this->mInputList = inputList; }
    void setSuccessCallBack(void (*callback)(TCheatHandler *)) {
        this->mSuccessCallback = callback;
    }
    void setFailureCallBack(void (*callback)(TCheatHandler *)) {
        this->mFailureCallback = callback;
    }
    void setGamePad(TMarioGamePad *gamePad) { this->mGamePad = gamePad; }

    constexpr bool isActive() const { return (this->mActive && this->mInputIndex < 0); }
    constexpr bool isDead() const { return !(this->mActive && this->mInputIndex < 0); }
    constexpr bool isInitialized() const { return (this->mGamePad != nullptr); }

    void advanceInput();
    void succeed();
    void fail();
    void reset();

private:
    void (*mSuccessCallback)(TCheatHandler *);
    void (*mFailureCallback)(TCheatHandler *);

    u16 *mInputList;
    TMarioGamePad *mGamePad;
    s8 mInputIndex;
    bool mActive;
};

#pragma region Implementation

TCheatHandler::TCheatHandler()
    : mInputList(nullptr), mSuccessCallback(nullptr), mFailureCallback(nullptr), mGamePad(nullptr),
      mActive(false), mInputIndex(0) {}

TCheatHandler::TCheatHandler(u16 *inputList, TMarioGamePad *gamepad,
                             void (*successCallback)(TCheatHandler *),
                             void (*failureCallback)(TCheatHandler *))
    : mInputList(inputList), mSuccessCallback(successCallback), mFailureCallback(failureCallback),
      mGamePad(gamepad), mActive(false), mInputIndex(0) {}

void TCheatHandler::advanceInput() {
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

void TCheatHandler::succeed() {
    mInputIndex = -1;
    mActive     = true;

    if (mSuccessCallback)
        mSuccessCallback(this);
}

void TCheatHandler::fail() {
    mInputIndex = -1;
    mActive     = false;

    if (mFailureCallback)
        mFailureCallback(this);
}

void TCheatHandler::reset() {
    mInputIndex = 0;
    mActive     = false;
}

#pragma endregion