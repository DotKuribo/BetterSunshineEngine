#include <Dolphin/mem.h>

#include <SMS/Manager/FlagManager.hxx>
#include <SMS/macros.h>

#include "module.hxx"

// static void resetGame(TFlagManager *flagManager) { memset(this + 0xE4, 0, 0xD); }

// 0x80294EF4
// extern -> SMS.cpp
static void resetStage(TFlagManager *flagManager) {
    constexpr size_t mainResetSize = 0x100;
    memset(((u8 *)&flagManager->Type1Flag) + 0xE4, 0, 0xD);
    memset(((u8 *)&flagManager->Type1Flag) + 0xF4, 0, mainResetSize);  // OG =
                                                                       // 0x18C
    memset(((u8 *)&flagManager->Type1Flag) + 0x11F, 0, 1);
}
SMS_PATCH_B(SMS_PORT_REGION(0x80294EF4, 0x8028CD0C, 0, 0), resetStage);