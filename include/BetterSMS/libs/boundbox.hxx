#pragma once

#include "constmath.hxx"
#include <Dolphin/MTX.h>
#include <Dolphin/OS.h>
#include <Dolphin/types.h>
#include <JSystem/JGeometry/JGMBox.hxx>
#include <SMS/MarioUtil/MathUtil.hxx>

enum class BoundingType : u8 {
    Box,
    Spheroid,
};

struct BoundingBox {
    TVec3f center, size, rotation;

    BoundingBox() = default;
    BoundingBox(const JGeometry::TBox<TVec3f> &box) {
        center   = box.center;
        size     = box.size;
        rotation = {0.0f, 0.0f, 0.0f};
    }
    BoundingBox(const JGeometry::TBox<TVec3f> &box, const TVec3f &rotation) {
        center         = box.center;
        size           = box.size;
        this->rotation = rotation;
    }
    BoundingBox(const TVec3f &center, const TVec3f &size) {
        this->center = center;
        this->size   = size;
    }
    BoundingBox(const TVec3f &center, const TVec3f &size, const TVec3f &rotation) {
        this->center   = center;
        this->size     = size;
        this->rotation = rotation;
    }

    TVec3f sample(f32 lx, f32 ly, f32 lz, f32 scale = 1.0f,
                  BoundingType type = BoundingType::Box) const {
        Mtx rot;
        MsMtxSetTRS(rot, 0, 0, 0, rotation.x, rotation.y, rotation.z, 1, 1, 1);

        // Calculate the point in the local space of the box.
        TVec3f local_point;
        local_point.x = (lx - 0.5f) * size.x;
        local_point.y = (ly - 0.5f) * size.y;
        local_point.z = (lz - 0.5f) * size.z;

        if (type == BoundingType::Box) {
            local_point.scale(scale);

            // Transform this point to the global space.
            TVec3f global_point;
            PSMTXMultVec(rot, local_point, global_point);
            global_point += center;

            return global_point;
        } else {
            // Step 3: Calculate the magnitude of the normalized point and normalize the point to
            // ensure it lies within the unit sphere.
            float magnitude = sqrtf(local_point.x * local_point.x + local_point.y * local_point.y +
                                    local_point.z * local_point.z);

            local_point.x /= magnitude;
            local_point.y /= magnitude;
            local_point.z /= magnitude;

            // Step 4: Scale the point by the half-lengths of the box to get the corresponding point
            // in the spheroid space.
            TVec3f spheroid_point;
            spheroid_point.x = local_point.x * (size.x * 0.5f);
            spheroid_point.y = local_point.y * (size.y * 0.5f);
            spheroid_point.z = local_point.z * (size.z * 0.5f);

            spheroid_point.scale(scale);

            // Transform this point to the global space.
            TVec3f global_point;
            PSMTXMultVec(rot, spheroid_point, global_point);
            global_point += center;

            return global_point;
        }
    }

    bool contains(const TVec3f &point, f32 scale = 1.0f,
                  BoundingType type = BoundingType::Box) const {
        Mtx rot, inv_rot;
        MsMtxSetTRS(rot, 0, 0, 0, rotation.x, rotation.y, rotation.z, 1, 1, 1);

        PSMTXInverse(rot, inv_rot);

        TVec3f local_point = point, inv_point;
        local_point -= center;
        PSMTXMultVec(inv_rot, local_point, inv_point);
        if (type == BoundingType::Box) {
            return (fabsf(inv_point.x) <= size.x * 0.5f * scale) &&
                   (fabsf(inv_point.y) <= size.y * 0.5f * scale) &&
                   (fabsf(inv_point.z) <= size.z * 0.5f * scale);
        } else {
            // Normalize the point's coordinates with respect to the half-lengths of the box.
            inv_point.x /= (size.x * 0.5f * scale);
            inv_point.y /= (size.y * 0.5f * scale);
            inv_point.z /= (size.z * 0.5f * scale);

            // The point is inside the spheroid if the sum of the squares of its normalized
            // coordinates is less than or equal to 1.
            return sqrtf((inv_point.x * inv_point.x + inv_point.y * inv_point.y +
                          inv_point.z * inv_point.z)) <= 1.0f;
        }
    }
};