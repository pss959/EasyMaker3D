#include "SG/PrimitiveShape.h"

#include <ion/math/transformutils.h>

#include "Math/Linear.h"

namespace SG {

void PrimitiveShape::AddFields() {
    AddField(scale_);
    AddField(rotation_);
    AddField(translation_);
    Shape::AddFields();
}

void PrimitiveShape::UpdateShapeSpec(ion::gfxutils::ShapeSpec &spec) {
    spec.scale       = scale_.GetValue();
    spec.rotation    = ion::math::RotationMatrixNH(rotation_.GetValue());
    spec.translation = Point3f(translation_.GetValue());
}

Matrix4f PrimitiveShape::GetMatrix() const {
    return GetTransformMatrix(GetScale() * Vector3f(1, 1, 1),
                              GetRotation(), GetTranslation());
}

Bounds PrimitiveShape::ComputeBounds() const {
    return TransformBounds(GetUntransformedBounds(), GetMatrix());
}

}  // namespace SG
