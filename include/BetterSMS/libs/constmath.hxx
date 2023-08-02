#pragma once

#include <Dolphin/math.h>
#include <Dolphin/types.h>

constexpr f32 angleToRadians(f32 a) { return (static_cast<f32>(M_PI) / 180.0f) * a; }
constexpr f64 angleToRadians(f64 a) { return (M_PI / 180.0) * a; }
constexpr f32 radiansToAngle(f32 r) { return (180.0f / static_cast<f32>(M_PI)) * r; }
constexpr f64 radiansToAngle(f64 r) { return (180.0 / M_PI) * r; }

template <typename T> constexpr T scaleLinearAtAnchor(T value, T scale, T anchor) {
    return (value * scale) + (anchor - scale);
}

template <typename T> constexpr T lerp(T a, T b, f32 f) { return a + f * (b - a); }

template <typename T> constexpr T clamp(T value, T min, T max) { return Clamp(value, min, max); }

constexpr f32 sigmoid(f32 x, f32 f, f32 r, f32 c, f32 b) {
    return f + ((r - f) / (1.0f + expf((b * -1.0f) * (x - c))));
}

constexpr f64 sigmoid(f64 x, f64 f, f64 r, f64 c, f64 b) {
    return f + ((r - f) / (1.0f + expf((b * -1.0f) * (x - c))));
}

constexpr f32 convertAngleS16ToFloat(s16 angle) { return static_cast<f32>(angle) / 182.04445f; }
constexpr s16 convertAngleFloatToS16(f32 angle) { return static_cast<s16>(angle * 182.04445f); }