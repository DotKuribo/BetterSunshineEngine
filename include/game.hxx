#pragma once

#include <Dolphin/types.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>
#include <SMS/game/Application.hxx>

namespace BetterSMS {
    namespace Game {
        typedef void (*BootCallback)(TApplication *);

        bool isOnBootRegistered(const char *name);
        bool registerOnBootCallback(const char *name, BootCallback cb);
        bool deregisterOnBootCallback(const char *name);
    }  // namespace Debug
};     // namespace BetterSMS