#pragma once

#include <SMS/Player/Mario.hxx>
#include <SMS/Player/Yoshi.hxx>

namespace BetterSMS {
    namespace Yoshi {
        bool isMounted(TYoshi *yoshi);
        bool isMounted(TMario *player);
        bool isGreenYoshi(TYoshi *yoshi);
        bool isGreenYoshi(TMario *player);
        bool isGreenYoshiMounted(TYoshi *yoshi);
        bool isGreenYoshiMounted(TMario *player);
        bool isGreenYoshiAscendingWater(TMario *player);
    }  // namespace Yoshi
}  // namespace BetterSMS