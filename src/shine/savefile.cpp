#include <Dolphin/mem.h>

#include <SMS/Manager/FlagManager.hxx>
#include <SMS/macros.h>

#include "module.hxx"

// static void resetGame(TFlagManager *flagManager) { memset(this + 0xE4, 0, 0xD); }

// 0x80294EF4
// extern -> SMS.cpp
static void resetGame(TFlagManager *flagManager) {
    constexpr size_t mainResetSize = 0x100;
    memset(((u8 *)flagManager) + 0xD0, 0, 0x21);
    memset(((u8 *)flagManager) + 0xF4, 0, mainResetSize);  // OG =
                                                                       // 0x18C
    memset(((u8 *)flagManager) + 0x11F, 0, 1);
}
SMS_PATCH_B(SMS_PORT_REGION(0x802950a0, 0, 0, 0), resetGame);

static void resetStage(TFlagManager *flagManager) {
    constexpr size_t mainResetSize = 0x100;

    memset(((u8 *)flagManager) + 0xE4, 0, 0xD);
    memset(((u8 *)flagManager) + 0xF4, 0, mainResetSize);  // OG =
                                                           // 0x18C
    memset(((u8 *)flagManager) + 0x11F, 0, 1);
}
SMS_PATCH_B(SMS_PORT_REGION(0x80294EF4, 0x8028CD0C, 0, 0), resetStage);

static void resetFirstStartExt(TFlagManager *flagManager) {
    memset(((u8 *)flagManager) + 0x1F4, 0, 0x8C);
    flagManager->correctFlag();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80293DE4, 0, 0, 0), resetFirstStartExt);

static void extendResetCard(TFlagManager *flagManager) {
    flagManager->resetStage();
    memset(((u8 *)flagManager) + 0x1F4, 0, 0x8C);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80295470, 0, 0, 0), extendResetCard);