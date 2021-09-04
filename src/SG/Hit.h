#pragma once

#include "Math/Types.h"
#include "SG/NodePath.h"
#include "SG/Typedefs.h"

namespace SG {

//! A Hit struct stores information about an intersection of a ray with a scene
//! graph.
struct Hit {
    //! Path to the intersected Node. This will be empty if there was no
    //! intersection.
    NodePath path;

    //! Shape within the Node that was intersected. This will be a null pointer
    //! if there was no intersection.
    ShapePtr shape;

    //! World-coordinate ray used to create the Hit.
    Ray      world_ray;

    //! Parametric distance of the intersection point along the ray. Note that
    //! if the ray direction is not normalized, this will not be a real
    //! distance.
    float    distance = 0;

    //! Intersection point in the coordinate system of the intersected node.
    Point3f  point{ 0, 0, 0 };

    //! Surface normal at the intersection point in the coordinate system of
    //! the intersected node.
    Vector3f normal{ 0, 0, 0 };

    //! If the intersected object is composed of triangles, these are the
    //! indices of the 3 points forming the intersected triangle. Otherwise,
    //! they are all -1.
    Vector3i indices{ -1, -1, -1 };

    //! Convenience function that indicates whether there was an intersection.
    bool IsValid() const { return ! path.empty(); }
};

}  // namespace SG
