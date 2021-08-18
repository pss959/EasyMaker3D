#pragma once

#include "Graph/Shape.h"
#include "Graph/Typedefs.h"

namespace Graph {

//! Box is a derived Shape that represents a box.
//!
//! \ingroup Graph
class Box : public Shape {
  public:
    Box(const ion::gfx::ShapePtr &ion_shape) : Shape(ion_shape) {}

    // XXXX
};

}  // namespace Graph
