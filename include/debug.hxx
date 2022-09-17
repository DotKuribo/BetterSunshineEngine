#pragma once

#include <Dolphin/types.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>
#include <SMS/game/MarDirector.hxx>

namespace BetterSMS {
    namespace Debug {
        typedef void (*InitCallback)(TMarDirector *);
        typedef void (*UpdateCallback)(TMarDirector *);
        typedef void (*DrawCallback)(TMarDirector *, J2DOrthoGraph *);

        bool isInitRegistered(const char *name);
        bool isUpdateRegistered(const char *name);
        bool isDrawRegistered(const char *name);
        bool registerInitCallback(const char *name, InitCallback cb);
        bool registerUpdateCallback(const char *name, UpdateCallback cb);
        bool registerDrawCallback(const char *name, DrawCallback cb);
        bool deregisterInitCallback(const char *name);
        bool deregisterUpdateCallback(const char *name);
        bool deregisterDrawCallback(const char *name);
    }  // namespace Debug
};     // namespace BetterSMS