#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "Math/Types.h"
#include "SG/Shape.h"

namespace SG {

//! Rectangle is a derived Shape that represents a rectangle.
class Rectangle : public Shape {
  public:
    typedef ion::gfxutils::PlanarShapeSpec::PlaneNormal PlaneNormal;

    virtual void AddFields() override;

    const Vector2f &  GetSize()        const { return size_;         }
    PlaneNormal       GetPlaneNormal() const { return plane_normal_; }

    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;

  protected:
    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateIonShape() override;

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<Vector2f>       size_{"size"};
    Parser::EnumField<PlaneNormal> plane_normal_{
        "plane_normal", PlaneNormal::kPositiveZ};
    //!@}
};

}  // namespace SG
