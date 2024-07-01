#pragma once

#include <Dolphin/types.h>
#include <SMS/System/Application.hxx>

#define CONTEXT_DIRECT_SETTINGS_MENU 10

namespace BetterSMS {
    namespace Application {
        // Return false for gameloop
        typedef bool (*ContextCallback)(TApplication *);

        bool registerContextCallback(u8 context, ContextCallback cb);
        void setIntroStage(u8 area, u8 episode);

        // This is true when the game is booted for the first time
        // the state is saved in the Better Sunshine Engine
        // module save on the memory card.
        bool isFirstBoot();
        void showSettingsOnFirstBoot(bool show_on_boot);

    }  // namespace Application
};     // namespace BetterSMS