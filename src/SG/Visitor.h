#pragma once

#include <functional>

#include "SG/NodePath.h"
#include "SG/Typedefs.h"

namespace SG {

//! Visitor is a base class for classes that visit all SG:Node instances in a
//! scene graph rooted by a given SG::Node. Virtual functions are invoked for
//! each node that is visited. The virtual functions indicate how to proceed by
//! returning a TraversalCode.
class Visitor {
  public:
    //! Traversal codes: a traversal function returns one of these for each
    //! Node traversed to indicate what to do.
    enum class TraversalCode {
        kContinue,  //!< Continue traversing normally,
        kPrune,     //!< Do not continue under the Node, but keep going.
        kStop,      //!< Stop traversing completely.
    };

    //! Traverses the subgraph rooted by the given Node, calling the functions
    //! for each Node encountered. This returns the TraversalCode returned by
    //! the call to VisitNode() for the last node visited.
    TraversalCode Visit(const NodePtr &root);

  protected:
    //! This function is invoked when a Node is visited during traversal. It is
    //! passed the current NodePath from the root to the visited node. It
    //! should return a TraversalCode indicating how to proceed. The default
    //! implementation does nothing but return TraversalCode::kContinue.
    virtual TraversalCode VisitNodeStart(const NodePath &path) {
        return TraversalCode::kContinue;
    }

    //! This function is invoked at the end of visiting a Node. If the Node has
    //! no children or if VisitNode() returned either TraversalCode::kPrune or
    //! TraversalCode::kStop for this Node, this is invoked immediately after
    //! VisitNode(). Otherwise, it is invoked after traversing the Node's
    //! children. The default implementation does nothing.
    virtual void VisitNodeEnd(const NodePath &path) {}

  private:
    NodePath cur_path_;  //!< Current traversal path.

    //! Recursive function that does the work for Visit().
    TraversalCode Visit_(const NodePtr &root);
};

}  // namespace SG
