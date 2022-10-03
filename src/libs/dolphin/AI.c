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

#if _AI_USE_C

u32 AIGetStreamSampleCount() { return *(volatile u32 *)0xCC006C08; }

void AIResetStreamSampleCount() {
    volatile u32 *StreamSampleCountFlag = (volatile u32 *)0xCC006C00;
    u32 sampleCount                     = *StreamSampleCountFlag;
    *StreamSampleCountFlag              = (sampleCount & ~0x20) | 0x20;
}

u32 AIGetStreamTrigger() { return *(volatile u32 *)0xCC006C0C; }
void AISetStreamTrigger(u32 trigger) { *(volatile u32 *)0xCC006C0C = trigger; }

#else

__attribute__((naked)) u32 AIGetStreamSampleCount() {
    __asm__ volatile("lis 3, 0xCC00          \n\t"
                     "addi 3, 3, 27648       \n\t"
                     "lwz 3, 0x0008 (3)      \n\t"
                     "blr                    \n\t");
}

__attribute__((naked)) void AIResetStreamSampleCount() {
    __asm__ volatile("lis 3, 0xCC00          \n\t"
                     "lwz 0, 0x6C00 (3)      \n\t"
                     "rlwinm 0, 0, 0, 27, 25 \n\t"
                     "ori 0, 0, 0x0020       \n\t"
                     "stw 0, 0x6C00 (3)      \n\t"
                     "blr                    \n\t");
}

__attribute__((naked)) u32 AIGetStreamTrigger() {
    __asm__ volatile("lis 3, 0xCC00          \n\t"
                     "addi 3, 3, 27648       \n\t"
                     "lwz 3, 0x000C (3)      \n\t"
                     "blr                    \n\t");
}

__attribute__((naked)) void AISetStreamTrigger(u32 trigger) {
    __asm__ volatile("lis 4, 0xCC00          \n\t"
                     "stw 3, 0x6C0C (4)      \n\t"
                     "blr                    \n\t");
}

#endif

AISCallback AIRegisterStreamCallback(AISCallback cb) {
    AISCallback prev = __AIS_Callback;

    u32 _atomic_state = OSDisableInterrupts();
    __AIS_Callback    = cb;
    OSRestoreInterrupts(_atomic_state);

    return prev;
}