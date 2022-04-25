#pragma once

#include "Math/Types.h"
#include "Memory.h"
#include "SG/Shape.h"

#include <ion/gfxutils/shapeutils.h>

namespace SG {

DECL_SHARED_PTR(PrimitiveShape);

/// PrimitiveShape is an abstract base class for Shapes that can have
/// transformation fields (uniform scale, rotation, and translation).
///
/// \ingroup SG
class PrimitiveShape : public Shape {
  public:
    /// \name Transformation Query Functions.
    ///@{
    float             GetScale()       const { return scale_;       }
    const Rotationf & GetRotation()    const { return rotation_;    }
    const Vector3f  & GetTranslation() const { return translation_; }
    ///@}

    /// Redefines this to compute a Ray in coordinates without the
    /// transformations applied and then call IntersectUntransformedRay().
    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;

  protected:
    virtual void AddFields() override;

    /// Updates the corresponding transformation fields in the given Ion
    /// ShapeSpec.
    void UpdateShapeSpec(ion::gfxutils::ShapeSpec &spec);

    /// Defines this to apply the transformation fields to the bounds returned
    /// by GetUntransformedBounds.
    virtual Bounds ComputeBounds() const;

    /// Derived classes must implement this to return the untransformed bounds.
    virtual Bounds GetUntransformedBounds() const = 0;

    /// Derived classes must implement this to intersect with the given
    /// untransformed ray.
    virtual bool IntersectUntransformedRay(const Ray &ray, Hit &hit) const = 0;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<float>     scale_{"scale", 1};
    Parser::TField<Rotationf> rotation_{"rotation"};
    Parser::TField<Vector3f>  translation_{"translation",{0, 0, 0}};
    ///@}

    /// Returns a matrix implementing all transformations.
    Matrix4f GetMatrix_() const;
};

}  // namespace SG
