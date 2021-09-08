#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Math/Types.h"
#include "SG/Typedefs.h"
#include "Util/General.h"

namespace SG {

//! A NodePath represents a path from a Node to a descendent node.
struct NodePath : public std::vector<NodePtr> {
    //! Default constructor.
    NodePath() {}

    //! Constructor that takes the root node.
    NodePath(const NodePtr &root) : std::vector<NodePtr>(1, root) {}

    //! Returns a NodePath representing a sub-path of this NodePath but ending
    //! at the given node. Asserts if the node is not in the path.
    NodePath GetSubPath(const Node &end_node) const;

    //! \name Coordinate Transforms.
    //! Each of these transforms a 3D point or vector between the local
    //! coordinate system at the tail of the path to or from the coordinate
    //! system at the root of the path.
    //!@{

    //! Transforms the given point from local coordinates at the tail of the
    //! path to coordinates at the root of the path.
    Point3f FromLocal(const Point3f &local_pt) const;

    //! Transforms the given vector from local coordinates at the tail of the
    //! path to coordinates at the root of the path.
    Vector3f FromLocal(const Vector3f &local_vec) const;

    //! Transforms the given point to local coordinates at the tail of the path
    //! from coordinates at the root of the path.
    Point3f ToLocal(const Point3f &pt) const;

    //! Transforms the given vector to local coordinates at the tail of the
    //! path from coordinates at the root of the path.
    Vector3f ToLocal(const Vector3f &vec) const;

    //!@}

    //! Searches upward in the path for a Node that is of the given type,
    //! returning it or a null pointer.
    template <typename T> std::shared_ptr<T> FindNodeUpwards() const {
        for (auto it = rbegin(); it != rend(); ++it) {
            std::shared_ptr<T> t = Util::CastToDerived<T>(*it);
            if (t)
                return t;
        }
        return std::shared_ptr<T>(nullptr);
    }

    //! Converts to a string to help with debugging.
    std::string ToString() const;
};

}  // namespace SG
