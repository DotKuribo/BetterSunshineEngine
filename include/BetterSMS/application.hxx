#pragma once

#include <Dolphin/types.h>
#include <SMS/System/Application.hxx>

namespace BetterSMS {
    namespace Application {
        // Return false for gameloop
        typedef bool (*ContextCallback)(TApplication *);

        bool isContextRegistered(u8 context);
        bool registerContextCallback(u8 context, ContextCallback cb);
        void deregisterContextCallback(u8 context);
        void setIntroStage(u8 area, u8 episode);
    }  // namespace Game
};     // namespace BetterSMS