#include <Dolphin/DVD.h>
#include <Dolphin/OS.h>
#include <Dolphin/types.h>

extern OSThreadQueue __DVDThreadQueue;

extern bool autoInvalidation;
extern DVDCommandBlock *executing;
extern bool PauseFlag;
extern bool PausingFlag;

extern void stateReady();
extern bool __DVDPushWaitingQueue(u32, DVDCommandBlock *cmdblock);

static bool issueCommand(u32 command, DVDCommandBlock *cmdblock) {
    if (autoInvalidation) {
        u32 curCommand = cmdblock->mCurCommand;
        if (curCommand == 1 || (curCommand - 4) <= 1 || curCommand == 14) {
            DCInvalidateRange(cmdblock->mAddress, cmdblock->mLength);
        }
    }
    u32 _atomic_state = OSDisableInterrupts();

    cmdblock->mCurState = DVD_STATE_WAITING;
    bool ret            = __DVDPushWaitingQueue(1, cmdblock);

    if (!executing && !PauseFlag)
        stateReady();

    OSRestoreInterrupts(_atomic_state);

    return ret;
}

static void cbForGetStreamErrorStatusSync(s32 result, DVDCommandBlock *cmdblock) {
    cmdblock->mCommandResult = result;
    OSWakeupThread(&__DVDThreadQueue);
}

static void cbForGetStreamLengthSync(s32 result, DVDCommandBlock *cmdblock) {
    cmdblock->mCommandResult = result;
    OSWakeupThread(&__DVDThreadQueue);
}

static void cbForGetStreamPlayAddrSync(s32 result, DVDCommandBlock *cmdblock) {
    cmdblock->mCommandResult = result;
    OSWakeupThread(&__DVDThreadQueue);
}

static void cbForGetStreamStartAddrSync(s32 result, DVDCommandBlock *cmdblock) {
    cmdblock->mCommandResult = result;
    OSWakeupThread(&__DVDThreadQueue);
}

static void cbForStopStreamAtEndSync(s32 result, DVDCommandBlock *cmdblock) {
    cmdblock->mCommandResult = result;
    OSWakeupThread(&__DVDThreadQueue);
}

bool DVDGetStreamErrorStatusAsync(DVDCommandBlock *cmdblock, DVDCBCallback cb) {
    cmdblock->mCurCommand = 9;
    cmdblock->mCB         = cb;
    return issueCommand(cmdblock->mCurCommand, cmdblock);
}

u32 DVDGetStreamErrorStatus(DVDCommandBlock *cmdblock) {
    cmdblock->mCurCommand = 9;
    cmdblock->mCB         = cbForGetStreamLengthSync;

    if (!issueCommand(cmdblock->mCurCommand, cmdblock))
        return -1;

    u32 _atomic_state = OSDisableInterrupts();

    while (cmdblock->mCurState != DVD_STATE_END && cmdblock->mCurState != DVD_STATE_CANCELED &&
           cmdblock->mCurState != DVD_STATE_FATAL_ERROR) {
        OSSleepThread(&__DVDThreadQueue);
    }

    OSRestoreInterrupts(_atomic_state);

    return cmdblock->mCommandResult;
}

bool DVDGetStreamLengthAsync(DVDCommandBlock *cmdblock, DVDCBCallback cb) {
    cmdblock->mCurCommand = 12;
    cmdblock->mCB         = cb;
    return issueCommand(cmdblock->mCurCommand, cmdblock);
}

u32 DVDGetStreamLength(DVDCommandBlock *cmdblock) {
    cmdblock->mCurCommand = 12;
    cmdblock->mCB         = cbForGetStreamLengthSync;

    if (!issueCommand(cmdblock->mCurCommand, cmdblock))
        return -1;

    u32 _atomic_state = OSDisableInterrupts();

    while (true) {
        if (cmdblock->mCurState == DVD_STATE_END || cmdblock->mCurState == DVD_STATE_CANCELED ||
            cmdblock->mCurState == DVD_STATE_FATAL_ERROR)
            break;
        OSSleepThread(&__DVDThreadQueue);
    }

    OSRestoreInterrupts(_atomic_state);

    return cmdblock->mCommandResult;
}

#ifdef SME_DVD_OVERRIDE
bool DVDGetStreamPlayAddrAsync(DVDCommandBlock *cmdblock, DVDCBCallback cb) {
    cmdblock->mCurCommand = 10;
    cmdblock->mCB         = cb;
    return issueCommand(cmdblock->mCurCommand, cmdblock);
}
#endif

u32 DVDGetStreamPlayAddr(DVDCommandBlock *cmdblock) {
    cmdblock->mCurCommand = 10;
    cmdblock->mCB         = cbForGetStreamPlayAddrSync;

    if (!issueCommand(cmdblock->mCurCommand, cmdblock))
        return -1;

    u32 _atomic_state = OSDisableInterrupts();

    while (cmdblock->mCurState != DVD_STATE_END && cmdblock->mCurState != DVD_STATE_CANCELED &&
           cmdblock->mCurState != DVD_STATE_FATAL_ERROR) {
        OSSleepThread(&__DVDThreadQueue);
    }

    OSRestoreInterrupts(_atomic_state);

    return cmdblock->mCommandResult;
}

bool DVDGetStreamStartAddrAsync(DVDCommandBlock *cmdblock, DVDCBCallback cb) {
    cmdblock->mCurCommand = 11;
    cmdblock->mCB         = cb;
    return issueCommand(cmdblock->mCurCommand, cmdblock);
}

u32 DVDGetStreamStartAddr(DVDCommandBlock *cmdblock) {
    cmdblock->mCurCommand = 11;
    cmdblock->mCB         = cbForGetStreamStartAddrSync;

    if (!issueCommand(cmdblock->mCurCommand, cmdblock))
        return -1;

    u32 _atomic_state = OSDisableInterrupts();

    while (cmdblock->mCurState != DVD_STATE_END && cmdblock->mCurState != DVD_STATE_CANCELED &&
           cmdblock->mCurState != DVD_STATE_FATAL_ERROR) {
        OSSleepThread(&__DVDThreadQueue);
    }

    OSRestoreInterrupts(_atomic_state);

    return cmdblock->mCommandResult;
}

u32 DVDGetTransferredSize(DVDCommandBlock *cmdblock) {
    if (cmdblock->mCurState == DVD_STATE_WAITING)
        return 0;

    if (cmdblock->mCurState == DVD_STATE_END || cmdblock->mCurState == DVD_STATE_FATAL_ERROR)
        return cmdblock->mCommandResult;

    if (cmdblock->mCurState == DVD_STATE_BUSY) {
        volatile u32 *DVDTransferredSize = (volatile u32 *)0xCC006018;
        return (cmdblock->mCurTransferSize - *DVDTransferredSize) + cmdblock->mCommandResult;
    }

    return (u32)cmdblock;
}

#ifdef SME_DVD_OVERRIDE
bool DVDStopStreamAtEndAsync(DVDCommandBlock *cmdblock, DVDCBCallback cb) {
    cmdblock->mCurCommand = 8;
    cmdblock->mCB         = cb;
    return issueCommand(cmdblock->mCurCommand, cmdblock);
}
#endif

u32 DVDStopStreamAtEnd(DVDCommandBlock *cmdblock) {
    cmdblock->mCurCommand = 8;
    cmdblock->mCB         = cbForGetStreamStartAddrSync;

    if (!issueCommand(cmdblock->mCurCommand, cmdblock))
        return -1;

    u32 _atomic_state = OSDisableInterrupts();

    while (cmdblock->mCurState != DVD_STATE_END && cmdblock->mCurState != DVD_STATE_CANCELED &&
           cmdblock->mCurState != DVD_STATE_FATAL_ERROR) {
        OSSleepThread(&__DVDThreadQueue);
    }

    OSRestoreInterrupts(_atomic_state);

    return cmdblock->mCommandResult;
}

void DVDPause() {
    u32 _atomic_state = OSDisableInterrupts();

    PauseFlag = true;
    if (!executing)
        PausingFlag = true;

    OSRestoreInterrupts(_atomic_state);
}

void DVDResume() {
    u32 _atomic_state = OSDisableInterrupts();

    PauseFlag = false;
    if (executing) {
        PausingFlag = false;
        stateReady();
    }

    OSRestoreInterrupts(_atomic_state);
}
