#pragma once

#include <JSystem/JDrama/JDRNameRef.hxx>
#include <Player/Mario.hxx>
#include <SMS/MapObj/MapObjInit.hxx>

namespace BetterSMS {

    namespace THP {

        bool addTHP(u8 id, const char *filename);
        const char *getTHP(u8 id);

    }  // namespace THP

}  // namespace BetterSMS
