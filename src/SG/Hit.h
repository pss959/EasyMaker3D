//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Ray.h"
#include "Math/Types.h"
#include "SG/CoordConv.h"
#include "SG/NodePath.h"

namespace SG {

DECL_SHARED_PTR(Shape);

/// A Hit struct stores information about an intersection of a ray with a scene
/// graph.
///
/// \ingroup SG
struct Hit {
    /// Path to the intersected Node. This will be empty if there was no
    /// intersection.
    NodePath path;

    /// Shape within the Node that was intersected. This will be a null pointer
    /// if there was no intersection.
    ShapePtr shape;

    /// World-coordinate ray used to create the Hit.
    Ray      world_ray;

    /// Parametric distance of the intersection point along the ray. Note that
    /// if the ray direction is not normalized, this will not be a real
    /// distance.
    float    distance = 0;

    /// Intersection point in the object coordinate system of the intersected
    /// node.
    Point3f  point{ 0, 0, 0 };

    /// Surface normal at the intersection point in the object coordinate
    /// system of the intersected node.
    Vector3f normal{ 0, 0, 0 };

    /// If the intersected object is composed of triangles, these are the
    /// indices of the 3 points forming the intersected triangle. Otherwise,
    /// they are all -1.
    Vector3i indices{ -1, -1, -1 };

    /// If the intersected object is composed of triangles, these are the
    /// barycentric coordinates for the 3 points forming the intersected
    /// triangle. Otherwise, they are all 0.
    Vector3f  barycentric{ 0, 0, 0 };

    /// Intersection point on the bounds of the intersected Node in the object
    /// coordinate system of that node.
    Point3f  bounds_point{ 0, 0, 0 };

    /// Convenience function that indicates whether there was an intersection.
    bool IsValid() const { return ! path.empty(); }

    /// Convenience that converts the point into world coordinates. There must
    /// be a valid path for this to work.
    Point3f  GetWorldPoint()  const {
        return CoordConv(path).ObjectToRoot(point);
    }

    /// Convenience that converts the normal into world coordinates. There must
    /// be a valid path for this to work.
    Vector3f GetWorldNormal() const {
        return CoordConv(path).ObjectToRoot(normal);
    }

    /// Equality operator.
    bool operator==(const Hit &h) const = default;
};

}  // namespace SG
