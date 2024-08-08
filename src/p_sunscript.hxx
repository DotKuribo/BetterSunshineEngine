#pragma once

#include <Dolphin/types.h>
#include <SMS/SPC/SpcInterp.hxx>
#include <SMS/SPC/SpcSlice.hxx>
#include <SMS/assert.h>

#include "sunscript.hxx"

namespace BetterSMS {

    namespace Spc {

        void spawnObjByID(TSpcInterp *interp, u32 argc);
        void getPlayerInputByIndex(TSpcInterp *interp, u32 argc);
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

}  // namespace BetterSMS

using namespace BetterSMS;