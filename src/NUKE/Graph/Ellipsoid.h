#pragma once

#include "Graph/Shape.h"
#include "Graph/Typedefs.h"

namespace Graph {

//! Ellipsoid is a derived Shape that represents a ellipsoid.
//!
//! \ingroup Graph
class Ellipsoid : public Shape {
  public:
    Ellipsoid(const ion::gfx::ShapePtr &ion_shape) : Shape(ion_shape) {}

    // XXXX
};

}  // namespace Graph
