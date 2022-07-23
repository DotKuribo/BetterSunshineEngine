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

u32 AIGetStreamSampleCount() {
    volatile u32 *StreamSampleCount = (volatile u32 *)0xCC006C08;
    return *StreamSampleCount;
}

void AIResetStreamSampleCount() {
    volatile u32 *StreamSampleCountFlag = (volatile u32 *)0xCC006C00;
    u32 sampleCount                     = *StreamSampleCountFlag;
    *StreamSampleCountFlag              = (sampleCount & ~0x20) | 0x20;
}

u32 AIGetStreamTrigger() {
    volatile u32 *StreamTrigger = (volatile u32 *)0xCC006C0C;
    return *StreamTrigger;
}

void AISetStreamTrigger(u32 trigger) {
    volatile u32 *StreamTrigger = (volatile u32 *)0xCC006C0C;
    *StreamTrigger              = trigger;
}

AISCallback AIRegisterStreamCallback(AISCallback cb) {
    AISCallback prev = __AIS_Callback;

    u32 _atomic_state = OSDisableInterrupts();
    __AIS_Callback    = cb;
    OSRestoreInterrupts(_atomic_state);

    return prev;
}