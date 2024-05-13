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
        static void normalToRotation(const TVec3f &norm, const TVec3f &up, Mtx &out) {
            TVec3f forward{};
            PSVECNormalize(norm, forward);

            TVec3f localup{};
            TVec3f right{};

            PSVECCrossProduct(up, forward, right);
            PSVECNormalize(right, right);
            PSVECCrossProduct(forward, right, localup);

            PSMTXIdentity(out);
            out[0][0] = right.x;
            out[0][1] = right.y;
            out[0][2] = right.z;

            out[1][0] = localup.x;
            out[1][1] = localup.y;
            out[1][2] = localup.z;

            out[2][0] = forward.x;
            out[2][1] = forward.y;
            out[2][2] = forward.z;
        }

        static void normalToRotationU(const TVec3f &norm, Mtx &out) {
            TVec3f forward{};
            PSVECNormalize(norm, forward);
            TVec3f up = fabsf(forward.y) < 0.999f ? TVec3f::up() : TVec3f::forward();
            normalToRotation(norm, up, out);
        }

        static void normalToRotationF(const TVec3f &norm, Mtx &out) {
            TVec3f forward{};
            PSVECNormalize(norm, forward);
            TVec3f up = fabsf(forward.y) > 0.001f ? TVec3f::forward() : TVec3f::up();
            normalToRotation(norm, up, out);
        }

        static float determinant(const Mtx& mtx) {
            return mtx[0][0] * (mtx[1][1] * mtx[2][2] - mtx[1][2] * mtx[2][1]) -
                   mtx[0][1] * (mtx[1][0] * mtx[2][2] - mtx[1][2] * mtx[2][0]) +
                   mtx[0][2] * (mtx[1][0] * mtx[2][1] - mtx[1][1] * mtx[2][0]);
        }

        static void decompose(const Mtx &mtx, TVec3f &translation, TVec3f &rotation,
                              TVec3f &scale) {
            scale.x = sqrtf(mtx[0][0] * mtx[0][0] + mtx[1][0] * mtx[1][0] + mtx[2][0] * mtx[2][0]);
            scale.y = sqrtf(mtx[0][1] * mtx[0][1] + mtx[1][1] * mtx[1][1] + mtx[2][1] * mtx[2][1]);
            scale.z = sqrtf(mtx[0][2] * mtx[0][2] + mtx[1][2] * mtx[1][2] + mtx[2][2] * mtx[2][2]);

            if (scale.x == 0.0f || scale.y == 0.0f || scale.z == 0.0f) {
                rotation.x = 0.0f;
                rotation.y = 0.0f;
                rotation.z = 0.0f;
                translation.x = mtx[0][3];
                translation.y = mtx[1][3];
                translation.z = mtx[2][3];
                return;
            }

            if (determinant(mtx) < 0.0f) {
                scale.x = -scale.x;
                scale.y = -scale.y;
                scale.z = -scale.z;
            }

            // Scale rotation matrix
            Mtx rot{};
            PSMTXCopy(mtx, rot);

            for (size_t i = 0; i < 3; i++) {
                rot[i][0] /= scale.x;
                rot[i][1] /= scale.y;
                rot[i][2] /= scale.z;
            }

            f32 sy = -rot[2][0];
            f32 y  = M_PI * 0.5f + acosf(sy);
            f32 cy = cosf(y);

            if (sy > 0.999f) {
                rotation.x = 0;
                rotation.y = -radiansToAngle(M_PI * 0.5f);
                rotation.z = radiansToAngle(atan2f(-rot[1][0], rot[1][1]));
            } else if (sy < -0.999f) {
                rotation.x = 0;
                rotation.y = -radiansToAngle(-M_PI * 0.5f);
                rotation.z = radiansToAngle(atan2f(-rot[1][0], rot[1][1]));
            } else {
                f32 cx = rot[2][2] / cy;
                f32 sx = rot[2][1] / cy;
                f32 cz = rot[0][0] / cy;
                f32 sz = rot[1][0] / cy;

                rotation.x = radiansToAngle(atan2f(sx, cx));
                rotation.y = radiansToAngle(y);
                rotation.z = radiansToAngle(atan2f(sz, cz));
            }

            translation.x = mtx[0][3];
            translation.y = mtx[1][3];
            translation.z = mtx[2][3];
        }
    };

    class Vector3 {
    public:
        static inline f32 magnitude(const TVec3f &vec) {
#if BETTER_SMS_USE_PS_MATH
            return PSVECMag(vec);
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
            PSVECNormalize(vec, out);
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
            return PSVECDotProduct(a, b);
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
            PSVECCrossProduct(a, b, out);
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

            f32 sy = -mtx[2][0];
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
                f32 cx = mtx[0][0] / cy;
                f32 sx = mtx[0][1] / cy;
                f32 cz = mtx[2][2] / cy;
                f32 sz = mtx[1][2] / cy;

                out.x = radiansToAngle(atan2f(-sx, cx));
                out.y = -radiansToAngle(y);
                out.z = radiansToAngle(atan2f(-sz, cz));
            }

            return out;
        }
    };

}  // namespace BetterSMS
