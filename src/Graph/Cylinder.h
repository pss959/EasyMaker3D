#pragma once

#include <ion/gfx/shape.h>

#include "Graph/Typedefs.h"

namespace Graph {

//! Cylinder is a derived Shape that represents a cylinder.
//! \ingroup Graph
class Cylinder : public Shape {
    // XXXX

  private:
    ion::gfx::ShapePtr  i_shape_;  //! Associated Ion Shape.
};

}  // namespace Graph
