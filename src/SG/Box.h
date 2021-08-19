#pragma once

#include "NParser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Shape.h"

namespace SG {

//! Box is a derived Shape that represents a box.
class Box : public Shape {
  public:
    //! Returns the 3D size of the box.
    const Vector3f & GetSize() const { return size_; }

    virtual ion::gfx::ShapePtr CreateIonShape() override;

    static NParser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    Vector3f size_{ 2.f, 2.f, 2.f };
    //!@}
};

}  // namespace SG
