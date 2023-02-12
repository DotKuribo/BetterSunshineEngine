#pragma once

#include <Dolphin/types.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>
#include <SMS/System/Application.hxx>

namespace BetterSMS {
    namespace Debug {
        typedef void (*InitCallback)(TApplication *);
        typedef void (*UpdateCallback)(TApplication *);
        typedef void (*DrawCallback)(TApplication *, const J2DOrthoGraph *);

        bool registerInitCallback(const char *name, InitCallback cb);
        bool registerUpdateCallback(const char *name, UpdateCallback cb);
        bool registerDrawCallback(const char *name, DrawCallback cb);
        void deregisterInitCallback(const char *name);
        void deregisterUpdateCallback(const char *name);
        void deregisterDrawCallback(const char *name);
    }  // namespace Debug
};     // namespace BetterSMS