#pragma once

#include <Dolphin/types.h>
#include <SMS/assert.h>
#include <SMS/SPC/SpcInterp.hxx>
#include <SMS/SPC/SpcSlice.hxx>

#include "sunscript.hxx"

namespace Spc {
    void vectorTranslate(TSpcInterp *interp, u32 argc);
    void vectorTranslatef(TSpcInterp *interp, u32 argc);
    void vectorScalef(TSpcInterp *interp, u32 argc);
    void vectorMagnitude(TSpcInterp *interp, u32 argc);
    void vectorNormalize(TSpcInterp *interp, u32 argc);
    void setActorPosToOther(TSpcInterp *interp, u32 argc);
    void setActorRotToOther(TSpcInterp *interp, u32 argc);
    void getActorPos(TSpcInterp *interp, u32 argc);
    void setActorPos(TSpcInterp *interp, u32 argc);
    void setActorPosf(TSpcInterp *interp, u32 argc);
    void getActorRot(TSpcInterp *interp, u32 argc);
    void setActorRot(TSpcInterp *interp, u32 argc);
    void setActorRotf(TSpcInterp *interp, u32 argc);
    void spawnObjByID(TSpcInterp *interp, u32 argc);
    void isDebugMode(TSpcInterp *interp, u32 argc);
    void getActivePlayers(TSpcInterp *interp, u32 argc);
    void getMaxPlayers(TSpcInterp *interp, u32 argc);
    void getPlayerByIndex(TSpcInterp *interp, u32 argc);
    void getDateAsStr(TSpcInterp *interp, u32 argc);
    void getTimeAsStr(TSpcInterp *interp, u32 argc);
    void getPlayerInputByIndex(TSpcInterp *interp, u32 argc);
    void read8(TSpcInterp *interp, u32 argc);
    void read16(TSpcInterp *interp, u32 argc);
    void read32(TSpcInterp *interp, u32 argc);
    void write8(TSpcInterp *interp, u32 argc);
    void write16(TSpcInterp *interp, u32 argc);
    void write32(TSpcInterp *interp, u32 argc);
    void memcpy_(TSpcInterp *interp, u32 argc);
    void memmove_(TSpcInterp *interp, u32 argc);
    void memcmp_(TSpcInterp *interp, u32 argc);
    void memset_(TSpcInterp *interp, u32 argc);
    void formatStrBySpec(TSpcInterp *interp, u32 argc);
    void getStageBGM(TSpcInterp *interp, u32 argc);
    void queueStream(TSpcInterp *interp, u32 argc);
    void playStream(TSpcInterp *interp, u32 argc);
    void pauseStream(TSpcInterp *interp, u32 argc);
    void stopStream(TSpcInterp *interp, u32 argc);
    void seekStream(TSpcInterp *interp, u32 argc);
    void nextStream(TSpcInterp *interp, u32 argc);
    void skipStream(TSpcInterp *interp, u32 argc);
    void getStreamVolume(TSpcInterp *interp, u32 argc);
    void setStreamVolume(TSpcInterp *interp, u32 argc);
    void getStreamLooping(TSpcInterp *interp, u32 argc);
    void setStreamLooping(TSpcInterp *interp, u32 argc);
}  // namespace Spc