#pragma once

#include <Dolphin/MTX.h>
#include <Dolphin/types.h>
#include <JSystem/JGeometry/JGMVec.hxx>

namespace BetterSMS {
    namespace Geometry {
        namespace Vector3 {
            f32 magnitude(const TVec3f &vec);
            f32 magnitude(const Vec &vec);

            f32 getNormalAngle(const TVec3f &vec);
            f32 getNormalAngle(const Vec &vec);

            void normalized(const TVec3f &vec, TVec3f &out);
            void normalized(const Vec &vec, Vec &out);

            f32 dot(const TVec3f &a, const TVec3f &b);
            f32 dot(const Vec &a, const Vec &b);

            void cross(const TVec3f &a, const TVec3f &b, TVec3f &out);
            void cross(const Vec &a, const Vec &b, Vec &out);

            f32 getYAngleTo(const TVec3f &a, const TVec3f &b);

            f32 lookAtRatio(const TVec3f &a, const TVec3f &b);
            f32 lookAtRatio(const Vec &a, const Vec &b);

            f32 angleBetween(const TVec3f &a, const TVec3f &b);
            f32 angleBetween(const Vec &a, const Vec &b);
        }  // namespace Vector3
    }      // namespace Geometry
}  // namespace BetterSMS::Geometry
