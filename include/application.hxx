#pragma once

#include <Dolphin/types.h>
#include <SMS/game/Application.hxx>

namespace BetterSMS {
    namespace Application {
        // Return false for gameloop
        typedef bool (*ContextCallback)(TApplication *);

        bool isContextRegistered(u8 context);
        bool registerContextCallback(u8 context, ContextCallback cb);
        bool deregisterContextCallback(u8 context);
    }  // namespace Game
};     // namespace BetterSMS