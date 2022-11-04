#pragma once

#include <Dolphin/types.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>
#include <SMS/System/Application.hxx>

namespace BetterSMS {
    namespace Game {
        typedef void (*InitCallback)(TApplication *);
        typedef void (*BootCallback)(TApplication *);
        typedef void (*LoopCallback)(TApplication *);
        typedef void (*ChangeCallback)(TApplication *);

        bool isOnInitRegistered(const char *name);
        bool isOnBootRegistered(const char *name);
        bool isOnLoopRegistered(const char *name);
        bool isOnChangeRegistered(const char *name);
        bool registerOnInitCallback(const char *name, InitCallback cb);
        bool registerOnBootCallback(const char *name, BootCallback cb);
        bool registerOnLoopCallback(const char *name, LoopCallback cb);
        bool registerOnChangeCallback(const char *name, ChangeCallback cb);
        bool deregisterOnInitCallback(const char *name);
        bool deregisterOnBootCallback(const char *name);
        bool deregisterOnLoopCallback(const char *name);
        bool deregisterOnChangeCallback(const char *name);
    }  // namespace Debug
};     // namespace BetterSMS