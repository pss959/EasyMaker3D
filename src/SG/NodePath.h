#pragma once

#include <memory>
#include <string>
#include <vector>

#include "SG/Typedefs.h"
#include "Util/General.h"

namespace SG {

//! A NodePath represents a path from a Node to a descendent node.
struct NodePath : public std::vector<NodePtr> {
    //! Default constructor.
    NodePath() {}

    //! Constructor that takes the root node.
    NodePath(const NodePtr &root) : std::vector<NodePtr>(1, root) {}

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
