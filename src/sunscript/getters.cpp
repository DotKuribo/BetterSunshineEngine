#include <SMS/Player/Mario.hxx>
#include <SMS/System/Application.hxx>

#include "p_sunscript.hxx"
#include "time.hxx"

void Spc::getTimeAsStr(TSpcInterp *interp, u32 argc) {}

void Spc::getPlayerInputByIndex(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(2, &argc);

    bool isFrameInput         = static_cast<bool>(Stack::popItem(interp).mValue);
    TMarioGamePad *controller = gpApplication.mGamePads[Stack::popItem(interp).mValue];
    if (isFrameInput)
        Stack::pushItem(interp, controller->mButtons.mFrameInput,
                        ValueType::INT);  // Return a value
    else
        Stack::pushItem(interp, controller->mButtons.mInput, ValueType::INT);  // Return a value
}