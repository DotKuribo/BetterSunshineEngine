#include <Dolphin/string.h>

#include <SMS/Manager/FlagManager.hxx>
#include <SMS/MapObj/MapObjTree.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include "module.hxx"

void ensureRedCoinsPanelAdjustOpen(TGCConsole2 *console, int unk_1, s32 time) {
    startAppearTimer__11TGCConsole2Fil(console, unk_1, time);
    if (console->mRedCoinPanel->mPane->mIsVisible)
        startAppearRedCoin__11TGCConsole2Fv(console);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8028A188, 0, 0, 0), ensureRedCoinsPanelAdjustOpen);
SMS_PATCH_BL(SMS_PORT_REGION(0x8028EB28, 0, 0, 0), ensureRedCoinsPanelAdjustOpen);
SMS_PATCH_BL(SMS_PORT_REGION(0x8028EB48, 0, 0, 0), ensureRedCoinsPanelAdjustOpen);

void ensureRedCoinsPanelAdjustClose(TGCConsole2 *console) {
    startDisappearTimer__11TGCConsole2Fv(console);
    if (console->mRedCoinPanel->mPane->mIsVisible) {
        console->mRedCoinPanelBack->mOffset.setValue(0x28, 0, 0, 0,
                                                     465 - console->mRedCoinPanelBack->mRect.mY1);
        console->mRedCoinPanelBack->mIsOffsetUpdating = true;
        console->mIsRedCoinCard                       = true;
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8028EB58, 0, 0, 0), ensureRedCoinsPanelAdjustClose);