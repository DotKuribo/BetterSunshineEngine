#include <Dolphin/AI.h>
#include <Dolphin/OS.h>
#include <Dolphin/types.h>

extern AISCallback __AIS_Callback;
extern AIDCallback __AID_Callback;
extern bool __AI_init_flag;
extern bool __AID_Active;

void AIReset() { __AI_init_flag = false; }

bool AICheckInit() { return __AI_init_flag; }

void AIStopDMA() {
    volatile u16 *DMAEnableFlag = (volatile u16 *)0xCC005036;
    *DMAEnableFlag &= ~0x8000;
}

u16 AIGetDMABytesLeft() {
    volatile u16 *DMABytes = (volatile u16 *)0xCC00503A;
    return ((*DMABytes) & 0x7FFF) << 5;
}

bool AIGetDMAEnableFlag() {
    volatile u16 *DMAEnableFlag = (volatile u16 *)0xCC005036;
    return ((*DMAEnableFlag) >> 15) & 1;
}

u16 AIGetDMALength() {
    volatile u16 *DMABytes = (volatile u16 *)0xCC005036;
    return ((*DMABytes) & 0x7FFF) << 5;
}

__attribute__((naked)) u16 AIGetDMAStartAddr() {
    __asm__ volatile("lis 3, 0xCC00          \n\t"
                     "addi 3, 3, 20480       \n\t"
                     "lhz 4, 0x30 (3)        \n\t"
                     "lhz 0, 0x32 (3)        \n\t"
                     "rlwinm 3, 0, 0, 16, 26 \n\t"
                     "rlwimi 3, 4, 16, 6, 15 \n\t"
                     "blr                    \n\t");
}

// These are injected into the DOL so that Nintendont prepatches them
u32 AIGetStreamSampleCount() { return ((u32(*)())0x80004000)(); }

void AIResetStreamSampleCount() { ((void (*)())0x80004010)(); }

u32 AIGetStreamTrigger() { return ((u32(*)())0x80004028)(); }

void AISetStreamTrigger(u32 trigger) { ((void (*)(u32))0x80004038)(trigger); }

AISCallback AIRegisterStreamCallback(AISCallback cb) {
    AISCallback prev = __AIS_Callback;

    u32 _atomic_state = OSDisableInterrupts();
    __AIS_Callback    = cb;
    OSRestoreInterrupts(_atomic_state);

    return prev;
}