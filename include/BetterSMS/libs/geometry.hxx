#pragma once

#include <Dolphin/MTX.h>
#include <Dolphin/math.h>
#include <Dolphin/types.h>
#include <JSystem/JGeometry/JGMVec.hxx>
#include <SMS/MarioUtil/MathUtil.hxx>

#include "../module.hxx"
#include "constmath.hxx"

namespace BetterSMS {

    class Matrix {
    public:
        static void rotateToNormal(const TVec3f &norm, Mtx &out) {
            TVec3f forward{};
            PSVECNormalize(norm, forward);

            TVec3f up = fabsf(forward.y) < 0.999f ? TVec3f::up() : TVec3f::forward();
            TVec3f right{};

            PSVECCrossProduct(up, forward, right);
            PSVECNormalize(right, right);
            PSVECCrossProduct(forward, right, up);

            PSMTXIdentity(out);
            out[0][0] = right.x;
            out[0][1] = right.y;
            out[0][2] = right.z;

            out[1][0] = up.x;
            out[1][1] = up.y;
            out[1][2] = up.z;

            out[2][0] = forward.x;
            out[2][1] = forward.y;
            out[2][2] = forward.z;
        }

    };

    class Vector3 {
    public:
        static inline f32 magnitude(const TVec3f &vec) {
#if BETTER_SMS_USE_PS_MATH
            return PSVECMag(reinterpret_cast<const Vec *>(&vec));
#else
            return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
#endif
        }

        static inline f32 magnitude(const Vec &vec) {
#if BETTER_SMS_USE_PS_MATH
            return PSVECMag(&vec);
#else
            return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
#endif
        }

        static inline f32 getNormalAngle(const TVec3f &vec) {
            return radiansToAngle(atan2f(vec.x, vec.z));
        }
        static inline f32 getNormalAngle(const Vec &vec) {
            return radiansToAngle(atan2f(vec.x, vec.z));
        }

        static inline void normalized(const TVec3f &vec, TVec3f &out) {
#if BETTER_SMS_USE_PS_MATH
            PSVECNormalize(reinterpret_cast<const Vec *>(&vec), reinterpret_cast<Vec *>(&out));
#else
            out.scale(magnitude(vec), vec);
#endif
        }

        static inline void normalized(const Vec &vec, Vec &out) {
#if BETTER_SMS_USE_PS_MATH
            PSVECNormalize(&vec, &out);
#else
            const f32 mag = magnitude(vec);
            out.x         = vec.x / mag;
            out.y         = vec.y / mag;
            out.z         = vec.z / mag;
#endif
        }

        static inline f32 dot(const TVec3f &a, const TVec3f &b) {
#if BETTER_SMS_USE_PS_MATH
            return PSVECDotProduct(reinterpret_cast<const Vec *>(&a),
                                   reinterpret_cast<const Vec *>(&b));
#else
            return a.x * b.x + a.y * b.y + a.z * b.z;
#endif
        }

        static inline f32 dot(const Vec &a, const Vec &b) {
#if BETTER_SMS_USE_PS_MATH
            return PSVECDotProduct(&a, &b);
#else
            return a.x * b.x + a.y * b.y + a.z * b.z;
#endif
        }

        static inline void cross(const TVec3f &a, const TVec3f &b, TVec3f &out) {
#if BETTER_SMS_USE_PS_MATH
            PSVECCrossProduct(reinterpret_cast<const Vec *>(&a), reinterpret_cast<const Vec *>(&b),
                              reinterpret_cast<Vec *>(&out));
#else
            out.x = a.y * b.z - a.z * b.y;
            out.y = a.z * b.x - a.x * b.z;
            out.z = a.x * b.y - a.y * b.x;
#endif
        }

        static inline void cross(const Vec &a, const Vec &b, Vec &out) {
#if BETTER_SMS_USE_PS_MATH
            PSVECCrossProduct(&a, &b, &out);
#else
            out.x = a.y * b.z - a.z * b.y;
            out.y = a.z * b.x - a.x * b.z;
            out.z = a.x * b.y - a.y * b.x;
#endif
        }

        static f32 getYAngleTo(const TVec3f &a, const TVec3f &b) {
            TVec3f diff = a;
            diff.sub(b);
            return MsGetRotFromZaxisY(diff);
        }

        static f32 lookAtRatio(const TVec3f &a, const TVec3f &b) {
            f32 angle = atan2f(b.z, -b.x) - atan2f(a.z, a.x);
            if (angle > M_PI) {
                angle -= 2 * M_PI;
            } else if (angle <= -M_PI) {
                angle += 2 * M_PI;
            }
            return fabsf(angle) / M_PI;
        }

        static f32 lookAtRatio(const Vec &a, const Vec &b) {
            f32 angle = atan2f(b.z, -b.x) - atan2f(a.z, a.x);
            if (angle > M_PI) {
                angle -= 2 * M_PI;
            } else if (angle <= -M_PI) {
                angle += 2 * M_PI;
            }
            return fabsf(angle) / M_PI;
        }

        static f32 angleBetween(const TVec3f &a, const TVec3f &b) {
            return dot(a, b) / (magnitude(a) * magnitude(b));
        }

        static f32 angleBetween(const Vec &a, const Vec &b) {
            return dot(a, b) / (magnitude(a) * magnitude(b));
        }

        static TVec3f eulerFromMatrix(Mtx mtx) {
            // Row-major rotation matrix
            // XYZ extrinsic rotation (ZYX intrinsic)
            // X -> left + clockwise
            // Y -> up + counter-clockwise
            // Z -> forward + clockwise

            TVec3f out{};

            f32 sy = mtx[0][2];
            f32 y  = M_PI * 0.5f - acosf(sy);
            f32 cy = cosf(y);

            if (sy > 0.999f) {
                out.x = 0;
                out.y = -radiansToAngle(M_PI * 0.5f);
                out.z = radiansToAngle(atan2f(-mtx[1][0], mtx[1][1]));
            } else if (sy < -0.999f) {
                out.x = 0;
                out.y = -radiansToAngle(-M_PI * 0.5f);
                out.z = radiansToAngle(atan2f(-mtx[1][0], mtx[1][1]));
            } else {
                f32 cx = mtx[2][2] / cy;
                f32 sx = mtx[1][2] / -cy;
                f32 cz = mtx[0][0] / cy;
                f32 sz = mtx[0][1] / -cy;

                out.x = radiansToAngle(atan2f(-sx, cx));
                out.y = -radiansToAngle(y);
                out.z = radiansToAngle(atan2f(-sz, cz));
            }

            return out;
        }

    };

}  // namespace BetterSMS
