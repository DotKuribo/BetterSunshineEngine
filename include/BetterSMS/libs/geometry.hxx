#pragma once

#include <Dolphin/math.h>
#include <Dolphin/MTX.h>
#include <Dolphin/types.h>
#include <JSystem/JGeometry/JGMVec.hxx>
#include <SMS/MarioUtil/MathUtil.hxx>

#include "constmath.hxx"
#include "module.hxx"

namespace BetterSMS {
    namespace Geometry {
        namespace Vector3 {
            inline f32 magnitude(const TVec3f &vec) {
#if BETTER_SMS_USE_PS_MATH
                return PSVECMag(reinterpret_cast<const Vec *>(&vec));
#else
                return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
#endif
            }

            inline f32 magnitude(const Vec &vec) {
#if BETTER_SMS_USE_PS_MATH
                return PSVECMag(&vec);
#else
                return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
#endif
            }

            inline f32 getNormalAngle(const TVec3f &vec) {
                return radiansToAngle(atan2f(vec.x, vec.z));
            }
            inline f32 getNormalAngle(const Vec &vec) {
                return radiansToAngle(atan2f(vec.x, vec.z));
            }

            inline void normalized(const TVec3f &vec, TVec3f &out) {
#if BETTER_SMS_USE_PS_MATH
                PSVECNormalize(reinterpret_cast<const Vec *>(&vec), reinterpret_cast<Vec *>(&out));
#else
                out.scale(magnitude(vec), vec);
#endif
            }

            inline void normalized(const Vec &vec, Vec &out) {
#if BETTER_SMS_USE_PS_MATH
                PSVECNormalize(&vec, &out);
#else
                const f32 mag = magnitude(vec);
                out.x         = vec.x / mag;
                out.y         = vec.y / mag;
                out.z         = vec.z / mag;
#endif
            }

            inline f32 dot(const TVec3f &a, const TVec3f &b) {
#if BETTER_SMS_USE_PS_MATH
                return PSVECDotProduct(reinterpret_cast<const Vec *>(&a),
                                       reinterpret_cast<const Vec *>(&b));
#else
                return a.x * b.x + a.y * b.y + a.z * b.z;
#endif
            }

            inline f32 dot(const Vec &a, const Vec &b) {
#if BETTER_SMS_USE_PS_MATH
                return PSVECDotProduct(&a, &b);
#else
                return a.x * b.x + a.y * b.y + a.z * b.z;
#endif
            }

            inline void cross(const TVec3f &a, const TVec3f &b, TVec3f &out) {
#if BETTER_SMS_USE_PS_MATH
                PSVECCrossProduct(reinterpret_cast<const Vec *>(&a),
                                  reinterpret_cast<const Vec *>(&b), reinterpret_cast<Vec *>(&out));
#else
                out.x = a.y * b.z - a.z * b.y;
                out.y = a.z * b.x - a.x * b.z;
                out.z = a.x * b.y - a.y * b.x;
#endif
            }

            inline void cross(const Vec &a, const Vec &b, Vec &out) {
#if BETTER_SMS_USE_PS_MATH
                PSVECCrossProduct(&a, &b, &out);
#else
                out.x = a.y * b.z - a.z * b.y;
                out.y = a.z * b.x - a.x * b.z;
                out.z = a.x * b.y - a.y * b.x;
#endif
            }

            inline f32 getYAngleTo(const TVec3f &a, const TVec3f &b) {
                TVec3f diff = a;
                diff.sub(b);
                return MsGetRotFromZaxisY(diff);
            }

            inline f32 lookAtRatio(const TVec3f &a, const TVec3f &b) {
                f32 angle = atan2f(b.z, -b.x) - atan2f(a.z, a.x);
                if (angle > M_PI) {
                    angle -= 2 * M_PI;
                } else if (angle <= -M_PI) {
                    angle += 2 * M_PI;
                }
                return fabsf(angle) / M_PI;
            }

            inline f32 lookAtRatio(const Vec &a, const Vec &b) {
                f32 angle = atan2f(b.z, -b.x) - atan2f(a.z, a.x);
                if (angle > M_PI) {
                    angle -= 2 * M_PI;
                } else if (angle <= -M_PI) {
                    angle += 2 * M_PI;
                }
                return fabsf(angle) / M_PI;
            }

            inline f32 angleBetween(const TVec3f &a, const TVec3f &b) {
                return dot(a, b) / (magnitude(a) * magnitude(b));
            }

            inline f32 angleBetween(const Vec &a, const Vec &b) {
                return dot(a, b) / (magnitude(a) * magnitude(b));
            }

        }  // namespace Vector3
    }      // namespace Geometry
}  // namespace BetterSMS::Geometry
