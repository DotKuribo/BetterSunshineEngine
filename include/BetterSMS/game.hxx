#pragma once

#include <Dolphin/types.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>
#include <SMS/System/Application.hxx>

namespace BetterSMS {
    namespace Game {
        size_t getMaxShines();
        void setMaxShines(size_t maxShines);

        typedef void (*InitCallback)(TApplication *);
        typedef void (*BootCallback)(TApplication *);
        typedef void (*LoopCallback)(TApplication *);
        typedef void (*DrawCallback)(TApplication *, const J2DOrthoGraph *);
        typedef void (*ChangeCallback)(TApplication *);

        bool registerInitCallback(const char *name, InitCallback cb);
        bool registerBootCallback(const char *name, BootCallback cb);
        bool registerLoopCallback(const char *name, LoopCallback cb);
        bool registerPostDrawCallback(const char *name, DrawCallback cb);
        bool registerChangeCallback(const char *name, ChangeCallback cb);
        void deregisterInitCallback(const char *name);
        void deregisterBootCallback(const char *name);
        void deregisterLoopCallback(const char *name);
        void deregisterPostDrawCallback(const char *name);
        void deregisterChangeCallback(const char *name);
    }  // namespace Game
};     // namespace BetterSMS