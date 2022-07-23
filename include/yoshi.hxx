#pragma once

#include <SMS/actor/Mario.hxx>
#include <SMS/actor/Yoshi.hxx>

namespace BetterSMS::Yoshi {
    bool isMounted(TYoshi *yoshi);
    bool isMounted(TMario *player);
    bool isGreenYoshi(TYoshi *yoshi);
    bool isGreenYoshi(TMario *player);
    bool isGreenYoshiMounted(TYoshi *yoshi);
    bool isGreenYoshiMounted(TMario *player);
    bool isGreenYoshiAscendingWater(TMario *player);
}  // namespace BetterSMS::Yoshi