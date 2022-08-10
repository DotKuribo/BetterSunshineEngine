#pragma once

#include <Dolphin/math.h>
#include <Dolphin/types.h>
#include <JSystem/JGeometry.hxx>
#include <SMS/macros.h>

namespace BetterSMS {
    namespace Math {
        /*
        / x = point on x axis
        / f = floor (min value)
        / r = roof (max value)
        / c = x offset
        / b = steepness
        /
        / Graphing Calculator: https://www.desmos.com/calculator/gfcphg11cn
        */
        f32 sigmoidCurve(f32 x, f32 f, f32 r, f32 c, f32 b);
        f64 sigmoidCurve(f64 x, f64 f, f64 r, f64 c, f64 b);
    }  // namespace Math
}  // namespace BetterSMS::Math
