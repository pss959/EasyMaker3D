#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "Math/Types.h"
#include "SG/PrimitiveShape.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(Rectangle);

/// Rectangle is a derived PrimitiveShape that represents a rectangle. It is
/// 1x1 in the Z=0 plane by default.
///
/// \ingroup SG
class Rectangle : public PrimitiveShape {
  public:
    using PlaneNormal = ion::gfxutils::PlanarShapeSpec::PlaneNormal;

    const Vector2f &  GetSize()        const { return size_;         }
    PlaneNormal       GetPlaneNormal() const { return plane_normal_; }

  protected:
    Rectangle() {}
    virtual void AddFields() override;
    virtual Bounds GetUntransformedBounds() const override;
    virtual bool IntersectUntransformedRay(const Ray &ray,
                                           Hit &hit) const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f>       size_;
    Parser::EnumField<PlaneNormal> plane_normal_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
