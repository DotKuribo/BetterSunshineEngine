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
    extern f32 sScreenWidth;
    extern f32 sFrameRate;

    bool isDebugMode();
    bool isMusicBeingStreamed();
    bool isMusicStreamingAllowed();
    void setDebugMode(bool active);
}  // namespace BetterSMS