#pragma once

#include "Graph/Shape.h"
#include "Graph/Typedefs.h"

namespace Graph {

//! Polygon is a derived Shape that represents a regular polygon.
//!
//! \ingroup Graph
class Polygon : public Shape {
  public:
    Polygon(const ion::gfx::ShapePtr &ion_shape) : Shape(ion_shape) {}

    // XXXX
};

}  // namespace Graph
