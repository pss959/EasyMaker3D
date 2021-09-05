#pragma once

#include "Math/Types.h"
#include "SG/Shape.h"

namespace SG {

//! Box is a derived Shape that represents a box.
class Box : public Shape {
  public:
    virtual void AddFields() override;

    //! Returns the 3D size of the box.
    const Vector3f & GetSize() const { return size_; }

    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;

  protected:
    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateIonShape() override;

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<Vector3f> size_{"size", { 2.f, 2.f, 2.f }};
    //!@}
};

}  // namespace SG
