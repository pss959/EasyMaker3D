#pragma once

#include "Graph/Shape.h"
#include "Graph/Typedefs.h"

namespace Graph {

//! Cylinder is a derived Shape that represents a cylinder.
//!
//! \ingroup Graph
class Cylinder : public Shape {
  public:
    Cylinder(const ion::gfx::ShapePtr &ion_shape) : Shape(ion_shape) {}
    // XXXX
};

}  // namespace Graph
