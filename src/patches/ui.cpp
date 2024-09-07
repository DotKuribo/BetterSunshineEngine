#include <Dolphin/string.h>

#include <SMS/G2D/BoundPane.hxx>
#include <SMS/G2D/ExPane.hxx>
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

static SMS_NO_INLINE void changeTextureForShineCounterOnSave_(u32 *card_save, J2DPicture *picture) {
    size_t shines = TFlagManager::smInstance->getFlag(0x40000);

    ResTIMG *timg =
        (ResTIMG *)((((u32 **)((u8 *)card_save + 0x1C))[(shines % 100) / 10])[0x20 / 4]);

    picture->changeTexture(timg, 0);
}

static void changeTextureForShineCounterOnSave_Tens_31(J2DPicture *picture) {
    u32 *card_save;
    SMS_FROM_GPR(31, card_save);

    changeTextureForShineCounterOnSave_(card_save, picture);
}
SMS_WRITE_32(SMS_PORT_REGION(0x8015E2A0, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015E2A4, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015E2A8, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015E2AC, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015E2B0, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015E2B4, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015E2B8, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015E2BC, 0, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x8015E2C0, 0, 0, 0), changeTextureForShineCounterOnSave_Tens_31);

static void changeTextureForShineCounterOnSave_Tens_30(J2DPicture *picture) {
    u32 *card_save;
    SMS_FROM_GPR(30, card_save);

    changeTextureForShineCounterOnSave_(card_save, picture);
}
SMS_WRITE_32(SMS_PORT_REGION(0x8015BFC4, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015BFC8, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015BFCC, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015BFD0, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015BFD4, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015BFD8, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015BFDC, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8015BFE0, 0, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x8015BFE4, 0, 0, 0), changeTextureForShineCounterOnSave_Tens_30);