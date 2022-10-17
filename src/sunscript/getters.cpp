#include <SMS/Player/Mario.hxx>
#include <SMS/System/Application.hxx>

#include "globals.hxx"
#include "sunscript.hxx"
#include "time.hxx"

void Spc::getTimeAsStr(TSpcInterp *interp, u32 argc) {}

void Spc::getPlayerInputByIndex(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(2, &argc);
    TMarioGamePad *controller = nullptr;
    switch (Stack::popItem(interp).mValue) {
    case 0:
        controller = gpApplication.mGamePad1;
    case 1:
        controller = gpApplication.mGamePad2;
    case 2:
        controller = gpApplication.mGamePad3;
    case 3:
        controller = gpApplication.mGamePad4;
    }
    bool isFrameInput = static_cast<bool>(Stack::popItem(interp).mValue);
    if (controller) {
        if (isFrameInput)
            Stack::pushItem(interp, controller->mButtons.mFrameInput,
                            ValueType::INT);  // Return a value
        else
            Stack::pushItem(interp, controller->mButtons.mInput, ValueType::INT);  // Return a value
    } else {
        SpcTrace("Controller not found!\n");
    }
}