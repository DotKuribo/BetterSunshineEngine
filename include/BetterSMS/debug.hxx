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

        bool addInitCallback(InitCallback cb);
        bool addUpdateCallback(UpdateCallback cb);
        bool addDrawCallback(DrawCallback cb);
    }  // namespace Debug
};     // namespace BetterSMS