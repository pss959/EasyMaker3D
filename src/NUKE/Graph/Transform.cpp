#include "Transform.h"

#include <ion/math/transformutils.h>

using ion::math::Matrix4f;
using ion::math::Rotationf;
using ion::math::Vector3f;

namespace Graph {

void Transform::SetScale(const Vector3f &scale) {
    scale_   = scale;
    any_set_ = true;
}

void Transform::SetRotation(const Rotationf &rotation) {
    rotation_ = rotation;
    any_set_  = true;
}

void Transform::SetTranslation(const Vector3f &translation) {
    translation_ = translation;
    any_set_     = true;
}

//! Returns the composite Matrix4f.
const Matrix4f & Transform::GetMatrix() const {
    if (any_set_) {
        matrix_ =
            ion::math::TranslationMatrix(translation_) *
            ion::math::RotationMatrixH(rotation_) *
            ion::math::ScaleMatrixH(scale_);
        any_set_ = false;
    }
    return matrix_;
}

}  // namespace Graph
