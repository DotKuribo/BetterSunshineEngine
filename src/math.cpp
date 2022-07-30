#include <Dolphin/math.h>
#include <Dolphin/types.h>
#include <SMS/macros.h>

#include "math.hxx"

using namespace BetterSMS;

SMS_NO_INLINE f32 Math::sigmoidCurve(f32 x, f32 f, f32 r, f32 c, f32 b) {
    return f + ((r - f) / (1.0f + expf((b * -1.0f) * (x - c))));
}

SMS_NO_INLINE f64 Math::sigmoidCurve(f64 x, f64 f, f64 r, f64 c, f64 b) {
    return f + ((r - f) / (1.0f + expf((b * -1.0f) * (x - c))));
}