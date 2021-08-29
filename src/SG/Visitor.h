#pragma once

#include <functional>

#include "SG/NodePath.h"
#include "SG/Typedefs.h"

namespace SG {

//! The Visitor class visits all SG:Node instances in the graph rooted by a
//! given SG::Node, calling a supplied function.
class Visitor {
  public:
    //! Traversal codes: a traversal function returns one of these for each
    //! Node traversed to indicate what to do.
    enum class TraversalCode {
        kContinue,  //!< Continue traversing normally,
        kPrune,     //!< Do not continue under the Node, but keep going.
        kStop,      //!< Stop traversing completely.
    };

    //! Typedef for traversal function. It is passed the current path from the
    //! root Node passed to Visit down to the current node.
    typedef std::function<TraversalCode(const NodePath &)> TraversalFunc;

    //! Traverses the subgraph rooted by the given Node, calling the given
    //! function for each Node encountered. The function should return one of
    //! the traversal codes to indicate what to do next. This returns the
    //! TraversalCode returned by the last node visited.
    TraversalCode Visit(const NodePtr &root, TraversalFunc func);

  private:
    //! Current traversal path.
    NodePath cur_path_;
};

}  // namespace SG
