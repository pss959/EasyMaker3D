#pragma once

#include <ion/gfx/shape.h>

#include "Graph/Object.h"
#include "Graph/Typedefs.h"

namespace Graph {

//! The Shape class is a base class for an object representing a shape in the
// scene graph. It allows for size and vertex queries among other things.
//!
//! \ingroup Graph
class Shape : public Object {
  public:
    // XXXX
    Shape(const ion::gfx::ShapePtr &ion_shape);

    //! Returns the associated Ion Shape.
    const ion::gfx::ShapePtr &GetIonShape() { return i_shape_; }

    // XXXX

  private:
    ion::gfx::ShapePtr  i_shape_;  //! Associated Ion Shape.

    //! Overrides this to also set the label in the Ion shape.
    virtual void SetName_(const std::string &name) override;

    friend class ::Input::Extractor;
};

}  // namespace Graph
