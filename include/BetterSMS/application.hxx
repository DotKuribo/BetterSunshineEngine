#pragma once

#include <Dolphin/types.h>
#include <SMS/System/Application.hxx>

namespace BetterSMS {
    namespace Application {
        // Return false for gameloop
        typedef bool (*ContextCallback)(TApplication *);

        bool registerContextCallback(u8 context, ContextCallback cb);
        void setIntroStage(u8 area, u8 episode);
    }  // namespace Application
};     // namespace BetterSMS