#pragma once

#include <ion/gfx/uniform.h>

#include "NParser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace SG {

//! A Uniform object wraps an Ion uniform.
class Uniform : public Object {
  public:
    //! Returns the associated Ion uniform.
    const ion::gfx::Uniform & GetIonUniform() const { return ion_uniform_; }

    static NParser::ObjectSpec GetObjectSpec();

  private:
    ion::gfx::Uniform ion_uniform_;  //! Associated Ion Uniform.

    // Parsed fields.
    float        float_val_;
    int          int_val;
    unsigned int uint_val_;
    Vector2f     vec2f_val_;
    Vector3f     vec3f_val_;
    Vector4f     vec4f_val_;
    Vector2i     vec2i_val_;
    Vector3i     vec3i_val_;
    Vector4i     vec4i_val_;
    Vector2ui    vec2ui_val_;
    Vector3ui    vec3ui_val_;
    Vector4ui    vec4ui_val_;
    Matrix2f     mat2_val_;
    Matrix3f     mat3_val_;
    Matrix4f     mat4_val_;

    //! Redefines this to set up the Ion Uniform.
    virtual void Finalize() override;
};

}  // namespace SG
