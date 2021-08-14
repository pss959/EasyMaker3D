#pragma once

#include <memory>

namespace Graph {

class Node;
class Shape;

//! Convenience typedef for a shared_ptr to a Node.
//! \ingroup Graph
typedef std::shared_ptr<Node>  NodePtr;

//! Convenience typedef for a shared_ptr to an Shape.
//! \ingroup Graph
typedef std::shared_ptr<Shape> ShapePtr;

}  // namespace Graph
