#pragma once

#include <Dolphin/types.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <SMS/game/MarDirector.hxx>

namespace BetterSMS {
    namespace Debug {
        typedef void (*DebugModeInitCallback)(TMarDirector *);
        typedef void (*DebugModeUpdateCallback)(TMarDirector *);
        typedef void (*DebugModeDrawCallback)(TMarDirector *, J2DOrthoGraph *);

        bool isDebugInitRegistered(const char *name);
        bool isDebugUpdateRegistered(const char *name);
        bool isDebugDrawRegistered(const char *name);
        bool registerDebugInitCallback(const char *name, DebugModeInitCallback cb);
        bool registerDebugUpdateCallback(const char *name, DebugModeUpdateCallback cb);
        bool registerDebugDrawCallback(const char *name, DebugModeDrawCallback cb);
        bool deregisterDebugInitCallback(const char *name);
        bool deregisterDebugUpdateCallback(const char *name);
        bool deregisterDebugDrawCallback(const char *name);
    }  // namespace Debug
};  // namespace BetterSMS