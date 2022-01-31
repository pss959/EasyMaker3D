#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "SG/Typedefs.h"
#include "Util/General.h"

namespace SG {

/// A NodePath represents a path from a Node to a descendent node.
struct NodePath : public std::vector<NodePtr> {
    /// Default constructor.
    NodePath() {}

    /// Constructor that takes the root node.
    NodePath(const NodePtr &root) : std::vector<NodePtr>(1, root) {}

    /// Returns a NodePath representing a sub-path of this NodePath with the
    /// same starting node but ending at the given node. Asserts if the node is
    /// not in the path.
    NodePath GetSubPath(const Node &end_node) const;

    /// Returns a NodePath representing a sub-path of this NodePath with the
    /// same ending node but starting at the given node. Asserts if the node is
    /// not in the path.
    NodePath GetEndSubPath(const Node &start_node) const;

    /// Stitches two NodePath instances together. The last node in p0 must be
    /// the same as the first node in p1.
    static NodePath Stitch(const NodePath &p0, const NodePath &p1);

    /// Converts to a string to help with debugging.
    std::string ToString() const;

    /// \name Path Searching
    ///@{

    /// Searches upward in the path for a Node that is of the given type,
    /// returning it or a null pointer.
    template <typename T> std::shared_ptr<T> FindNodeUpwards() const {
        for (auto it = rbegin(); it != rend(); ++it) {
            std::shared_ptr<T> t = Util::CastToDerived<T>(*it);
            if (t)
                return t;
        }
        return std::shared_ptr<T>(nullptr);
    }

    /// Searches upward in the path for a Node that returns true for the given
    /// predicate, returning it or a null pointer.
    NodePtr FindNodeUpwards(
        const std::function<bool(const Node &node)> &pred) const;

    ///@}
};

}  // namespace SG
