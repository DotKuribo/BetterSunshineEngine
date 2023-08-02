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
        if (type == BoundingType::Box) {
            Mtx rot;
            MsMtxSetTRS(rot, 0, 0, 0, rotation.x, rotation.y, rotation.z, 1, 1, 1);

            // Calculate the point in the local space of the box.
            TVec3f local_point;
            local_point.x = (lx - 0.5f) * size.x * scale;
            local_point.y = (ly - 0.5f) * size.y * scale;
            local_point.z = (lz - 0.5f) * size.z * scale;

            // Transform this point to the global space.
            TVec3f global_point;
            PSMTXMultVec(rot, local_point, global_point);
            global_point += center;

            return global_point;
        } else {
            // TODO: Calculate spheroid space
            return center;
        }
    }

    bool contains(const TVec3f &point, f32 scale = 1.0f,
                  BoundingType type = BoundingType::Box) const {
        if (type == BoundingType::Box) {
            Mtx rot, inv_rot;
            MsMtxSetTRS(rot, 0, 0, 0, rotation.x, rotation.y, rotation.z, 1, 1, 1);

            PSMTXInverse(rot, inv_rot);

            TVec3f local_point = point, inv_point;
            local_point -= center;
            PSMTXMultVec(inv_rot, local_point, inv_point);

            OSReport("X: %.02f, Y: %.02f, Z: %.02f\n", inv_point.x, inv_point.y, inv_point.z);

            return (fabsf(inv_point.x) <= size.x * 0.5f * scale) &&
                   (fabsf(inv_point.y) <= size.y * 0.5f * scale) &&
                   (fabsf(inv_point.z) <= size.z * 0.5f * scale);
        } else {
            // TODO: Calculate spheroid space
            return false;
        }
    }
};