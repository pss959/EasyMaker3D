#pragma once

#include <ion/gfx/shape.h>

#include "NParser/FieldSpec.h"
#include "SG/Object.h"

namespace SG {

//! A Shape object wraps an Ion Shape. Derived classes create specific types of
//! shapes.
class Shape : public Object {
  public:
    //! Returns the associated Ion shape.
    const ion::gfx::ShapePtr & GetIonShape() const { return ion_shape_; }

    static std::vector<NParser::FieldSpec> GetFieldSpecs();

  protected:
    //! Allows derived classes to set the Ion Shape.
    void SetShape(const ion::gfx::ShapePtr &shape) { ion_shape_ = shape; }

  private:
    ion::gfx::ShapePtr ion_shape_;  //! Associated Ion Shape.
};

}  // namespace SG
