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

        bool addInitCallback(InitCallback cb);
        bool addBootCallback(BootCallback cb);
        bool addLoopCallback(LoopCallback cb);
        bool addPostDrawCallback(DrawCallback cb);
        bool addChangeCallback(ChangeCallback cb);
    }  // namespace Game
};     // namespace BetterSMS