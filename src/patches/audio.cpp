#include <Dolphin/OS.h>
#include <Dolphin/types.h>

#include <SMS/macros.h>

#include "module.hxx"

struct JAIPlayerParameter {
    void *mTrack;
    u32 mArgs[10];  // TPortArgs, mArgs[1] holds the pending update flags
    void *mCmdHead;  // Non-null while the port command is queued
    u8 _30[0xC];
};
static_assert(sizeof(JAIPlayerParameter) == 0x3C);

struct JAISeqUpdateData {
    u8 _00[0x4C];
    JAIPlayerParameter *mPlayerParams;
};

// The audio thread may not have consumed the previous flags yet. Overwriting them
// can drop the tempo bit of a starting sequence, which then stays stuck on its first note.
static void setSeqPortargsU32(JAISeqUpdateData *data, u32 track, u8 arg, u32 value) {
    JAIPlayerParameter &param = data->mPlayerParams[track];

    const bool enable = OSDisableInterrupts();
    if (arg == 1 && param.mCmdHead)
        value |= param.mArgs[1];
    param.mArgs[arg] = value;
    OSRestoreInterrupts(enable);
}
SMS_PATCH_B(SMS_PORT_REGION(0x8030D330, 0, 0, 0), setSeqPortargsU32);

// Re-adding a command that is still queued drops every command queued after it.
SMS_WRITE_32(SMS_PORT_REGION(0x80307CF0, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8030669C, 0, 0, 0), 0x60000000);
