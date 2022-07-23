#pragma once

#include <dolphin/types.h>

#include "collision/warp.hxx"

namespace BetterSMS {
    static bool isDebugMode() { return _sIsDebugMode; }
    static bool isMusicBeingStreamed() { return _sIsAudioStreaming; }
    static bool isMusicStreamingAllowed() { return _sIsAudioStreamAllowed; }

    static void setDebugMode(bool active) { _sIsDebugMode = active; }

    static void *sPRMFile;
    static Collision::TWarpCollisionList *sWarpColArray;
    static Collision::TWarpCollisionList *sWarpColPreserveArray;

    static bool _sIsAudioStreaming;
    static bool _sIsAudioStreamAllowed;
    static bool _sIsDebugMode;
}  // namespace BetterSMS