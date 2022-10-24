#pragma once

#include <Dolphin/types.h>

#include "collision/warp.hxx"

namespace BetterSMS {
    extern void *sPRMFile;
    extern Collision::TWarpCollisionList *sWarpColArray;
    extern Collision::TWarpCollisionList *sWarpColPreserveArray;
    extern bool sIsAudioStreaming;
    extern bool sIsAudioStreamAllowed;
    extern bool sIsDebugMode;
    extern u32 sScreenWidth;
    extern f32 sFrameRate;
}  // namespace BetterSMS