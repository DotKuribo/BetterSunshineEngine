#pragma once

#include <Dolphin/MTX.h>
#include <Dolphin/math.h>
#include <Dolphin/types.h>
#include <JSystem/JGeometry/JGMVec.hxx>
#include <SMS/MarioUtil/MathUtil.hxx>

#include "../module.hxx"
#include "constmath.hxx"

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

            inline TVec3f eulerFromMatrix(Mtx rotMtx) {
                TVec3f euler;

                // Assuming the order is Yaw-Pitch-Roll (Y-X-Z)
                //if (rotMtx[0][2] > 0.998f) {  // singularity at north pole
                //    euler.x = atan2f(rotMtx[2][0], rotMtx[0][0]);
                //    euler.y = M_PI / 2;
                //    euler.z = 0;
                //} else if (rotMtx[0][2] < -0.998f) {  // singularity at south pole
                //    euler.x = atan2f(rotMtx[2][0], rotMtx[0][0]);
                //    euler.y = -M_PI / 2;
                //    euler.z = 0;
                //} else {
                //    euler.x = atan2f(-rotMtx[2][1], rotMtx[1][1]);
                //    euler.y = (M_PI / 2) - acosf(rotMtx[0][1]);
                //    euler.z = atan2f(-rotMtx[0][2], rotMtx[2][2]);
                //}

                //// Yaw (y rotation)
                //float sinY = -rotMtx[2][0];
                //float cosY = sqrtf(rotMtx[0][0] * rotMtx[0][0] + rotMtx[1][0] * rotMtx[1][0]);
                //euler.y    = atan2f(sinY, cosY);

                //// Pitch (x rotation)
                //euler.x = atan2f(rotMtx[1][2], -rotMtx[2][2]);

                //// Roll (z rotation)
                //euler.z = atan2f(-rotMtx[1][0], -rotMtx[0][0]);

                const float PI_OVER_2 = 1.57079632679f;  // Pi/2
                const float EPSILON   = 0.0001f;         // For precision checking
                float pitchTest       = -rotMtx[0][2];

                euler.x = atan2f(-rotMtx[2][1], rotMtx[2][2]);

                // yaw (y rotation)
                float c2 = sqrtf(rotMtx[0][0] * rotMtx[0][0] + rotMtx[1][0] * rotMtx[1][0]);
                euler.y  = atan2f(rotMtx[2][0], c2);

                // roll (z rotation)
                float s1 = sinf(euler.x);
                float c1 = cosf(euler.x);
                euler.z  = atan2f(s1 * rotMtx[0][2] - c1 * rotMtx[0][1],
                                  c1 * rotMtx[1][1] - s1 * rotMtx[1][2]);

                // Convert back to degrees
                euler.x = radiansToAngle(euler.x);
                euler.y = radiansToAngle(euler.y);
                euler.z = radiansToAngle(euler.z);

                return euler;
            }

        }  // namespace Vector3
    }      // namespace Geometry
}  // namespace BetterSMS
