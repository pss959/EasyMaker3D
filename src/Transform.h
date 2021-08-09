#pragma once

#include <ion/math/matrix.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>

//! A Transform instance represents a 3D scale/rotate/translate
//! transformation, converting to an Ion Matrix4f when needed.
class Transform {
  public:
    //! Sets the scale component.
    void SetScale(const ion::math::Vector3f &scale);

    //! Sets the rotation component.
    void SetRotation(const ion::math::Rotationf &rotation);

    //! Sets the translation component.
    void SetTranslation(const ion::math::Vector3f &translation);

    //! Returns the composite Matrix4f.
    const ion::math::Matrix4f & GetMatrix();

    //! Returns true if any of the components were changed, meaning that the
    //! matrix is not known to be identity.
    bool AnyComponentSet() const { return any_set_; }

  private:
    ion::math::Vector3f  scale_{ 1, 1, 1 };        //!< Scale component.
    ion::math::Rotationf rotation_;                //!< Rotation component.
    ion::math::Vector3f  translation_{ 0, 0, 0 };  //!< Translation component.

    // Composite matrix, recomputed when necessary.
    ion::math::Matrix4f  matrix_ = ion::math::Matrix4f::Identity();

    //! This gets set to true when any component value is set since
    //! construction or the last time GetMatrix() was called.
    bool any_set_ = false;
};

