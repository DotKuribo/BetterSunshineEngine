#pragma once

#include "area.hxx"

namespace BetterSMS {

    namespace Stage {

        struct NormalAreaInfo {
            s32 mShineStageID;
        };

        struct ExAreaInfo {
            s32 mShineStageID;
            s32 mShineID;
        };

        ShineAreaInfo **getShineAreaInfos();
        NormalAreaInfo *getNormalAreaInfos();
        ExAreaInfo *getExAreaInfos();

    }  // namespace Stage

}  // namespace BetterSMS