#pragma once

#include "Math/Types.h"
#include "SG/Shape.h"

#include <ion/gfxutils/shapeutils.h>

namespace SG {

/// PrimitiveShape is an abstract base class for Shapes that can have
/// transformation fields (uniform scale, rotation, and translation).
class PrimitiveShape : public Shape {
  public:
    virtual void AddFields() override;

    /// \name Transformation Query Functions.
    ///@{
    float             GetScale()       const { return scale_;       }
    const Rotationf & GetRotation()    const { return rotation_;    }
    const Vector3f  & GetTranslation() const { return translation_; }
    ///@}

  protected:
    /// Updates the corresponding transformation fields in the given Ion
    /// ShapeSpec.
    void UpdateShapeSpec(ion::gfxutils::ShapeSpec &spec);

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<float>     scale_{"scale", 1};
    Parser::TField<Rotationf> rotation_{"rotation"};
    Parser::TField<Vector3f>  translation_{"translation",{0, 0, 0}};
    ///@}
};

}  // namespace SG
