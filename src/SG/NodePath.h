#pragma once

#include <string>
#include <vector>

#include "SG/Typedefs.h"

namespace SG {

//! A NodePath represents a path from a Node to a descendent node.
struct NodePath : public std::vector<NodePtr> {
    //! Default constructor.
    NodePath() {}

    //! Constructor that takes the root node.
    NodePath(const NodePtr &root) : std::vector<NodePtr>(1, root) {}

    //! Converts to a string to help with debugging.
    std::string ToString() const;
};

}  // namespace SG
