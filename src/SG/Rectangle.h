#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "Parser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Shape.h"

namespace SG {

//! Rectangle is a derived Shape that represents a rectangle.
class Rectangle : public Shape {
  public:
    typedef ion::gfxutils::PlanarShapeSpec::PlaneNormal PlaneNormal;

    const Vector2f &  GetSize()        const { return size_;         }
    PlaneNormal       GetPlaneNormal() const { return plane_normal_; }

    virtual Bounds ComputeBounds() const override;
    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;
    virtual ion::gfx::ShapePtr CreateIonShape() override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    Vector2f    size_{ 2.f, 2.f };
    PlaneNormal plane_normal_ = PlaneNormal::kPositiveZ;
    //!@}
};

}  // namespace SG
