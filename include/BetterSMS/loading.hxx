#pragma once

#include <Dolphin/types.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>
#include <JSystem/JUtility/JUTTexture.hxx>
#include <SMS/System/MarDirector.hxx>

namespace BetterSMS {
    namespace Loading {
        void setLoading(bool isLoading);
        void setLoadingIcon(const ResTIMG **textures, size_t texCount);
        void setLayout(J2DScreen *screen);
        void setFrameRate(f32 fps);
    }  // namespace Loading
}  // namespace BetterSMS