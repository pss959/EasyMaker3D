#pragma once

#include <ion/gfx/shape.h>

#include "Parser/ObjectSpec.h"
#include "SG/Object.h"

namespace SG {

//! Abstract base class for all shapes; it wraps an Ion Shape. Derived classes
//! create specific types.
class Shape : public Object {
  public:
    //! Returns the associated Ion shape.
    const ion::gfx::ShapePtr & GetIonShape() const { return ion_shape_; }

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  protected:
    //! The constructor is protected to make this abstract.
    Shape() {}

    //! Derived classes must implement this to create the Ion Shape when
    //! necessary.
    virtual ion::gfx::ShapePtr CreateIonShape() = 0;

  private:
    ion::gfx::ShapePtr ion_shape_;  //! Associated Ion Shape.
};

}  // namespace SG
