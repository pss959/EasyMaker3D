#include "SG/PrimitiveShape.h"

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>

#include "Math/Linear.h"
#include "SG/Hit.h"

namespace SG {

void PrimitiveShape::AddFields() {
    AddField(scale_.Init("scale", 1));
    AddField(rotation_.Init("rotation"));
    AddField(translation_.Init("translation", Vector3f::Zero()));

    Shape::AddFields();
}

bool PrimitiveShape::IntersectRay(const Ray &ray, Hit &hit) const {
    const Matrix4f m = GetMatrix_();
    const Matrix4f inv_m = ion::math::Inverse(m);
    const bool intersected =
        IntersectUntransformedRay(TransformRay(ray, inv_m), hit);
    if (intersected) {
        // Apply the matrix to the results.
        hit.point  = m * hit.point;
        hit.normal = ion::math::Transpose(inv_m) * hit.normal;
    }
    return intersected;
}

void PrimitiveShape::UpdateShapeSpec(ion::gfxutils::ShapeSpec &spec) {
    spec.scale       = scale_.GetValue();
    spec.rotation    = ion::math::RotationMatrixNH(rotation_.GetValue());
    spec.translation = Point3f(translation_.GetValue());
}

Bounds PrimitiveShape::ComputeBounds() const {
    return TransformBounds(GetUntransformedBounds(), GetMatrix_());
}

Matrix4f PrimitiveShape::GetMatrix_() const {
    return GetTransformMatrix(GetScale() * Vector3f(1, 1, 1),
                              GetRotation(), GetTranslation());
}

}  // namespace SG
