#pragma once

#include "Graph/Shape.h"
#include "Graph/Typedefs.h"

namespace Graph {

//! Rectangle is a derived Shape that represents a rectangle.
//!
//! \ingroup Graph
class Rectangle : public Shape {
  public:
    Rectangle(const ion::gfx::ShapePtr &ion_shape) : Shape(ion_shape) {}

    // XXXX
};

}  // namespace Graph
