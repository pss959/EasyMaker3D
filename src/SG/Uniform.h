#pragma once

#include <ion/gfx/uniform.h>

#include "Math/Types.h"
#include "Parser/ObjectSpec.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/uniform.h>

namespace SG {

//! A Uniform object wraps an Ion uniform.
class Uniform : public Object {
  public:
    //! Returns the associated Ion uniform.
    const ion::gfx::Uniform & GetIonUniform() const { return ion_uniform_; }

    //! Returns the count of values. A count greater than 1 creates an array
    //! uniform.
    int GetCount() const { return count_; }

    //! Returns the name of the last field whose value was set. This will be
    //! empty if none was set.
    const std::string & GetLastFieldSet() const { return last_field_set_; }

    //! \name Value Access
    //! Only one of these will return the actual value of the uniform,
    //! depending on what GetLastFieldSet() returns.
    //!@{
    float             GetFloat()     const { return float_val_;  }
    int               GetInt()       const { return int_val_;    }
    unsigned int      GetUInt()      const { return uint_val_;   }
    const Vector2f  & GetVector2f()  const { return vec2f_val_;  }
    const Vector3f  & GetVector3f()  const { return vec3f_val_;  }
    const Vector4f  & GetVector4f()  const { return vec4f_val_;  }
    const Vector2i  & GetVector2i()  const { return vec2i_val_;  }
    const Vector3i  & GetVector3i()  const { return vec3i_val_;  }
    const Vector4i  & GetVector4i()  const { return vec4i_val_;  }
    const Vector2ui & GetVector2ui() const { return vec2ui_val_; }
    const Vector3ui & GetVector3ui() const { return vec3ui_val_; }
    const Vector4ui & GetVector4ui() const { return vec4ui_val_; }
    const Matrix2f  & GetMatrix2f()  const { return mat2_val_;   }
    const Matrix3f  & GetMatrix3f()  const { return mat3_val_;   }
    const Matrix4f  & GetMatrix4f()  const { return mat4_val_;   }
    //!@}

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    ion::gfx::Uniform ion_uniform_;  //! Associated Ion Uniform.

    //! \name Parsed Fields
    //!@{
    int          count_      = 1;
    float        float_val_  = 0.f;
    int          int_val_    = 0;
    unsigned int uint_val_   = 0;
    Vector2f     vec2f_val_  = Vector2f::Zero();
    Vector3f     vec3f_val_  = Vector3f::Zero();
    Vector4f     vec4f_val_  = Vector4f::Zero();
    Vector2i     vec2i_val_  = Vector2i::Zero();
    Vector3i     vec3i_val_  = Vector3i::Zero();
    Vector4i     vec4i_val_  = Vector4i::Zero();
    Vector2ui    vec2ui_val_ = Vector2ui::Zero();
    Vector3ui    vec3ui_val_ = Vector3ui::Zero();
    Vector4ui    vec4ui_val_ = Vector4ui::Zero();
    Matrix2f     mat2_val_   = Matrix2f::Identity();
    Matrix3f     mat3_val_   = Matrix3f::Identity();
    Matrix4f     mat4_val_   = Matrix4f::Identity();
    //!@}

    //! Stores the name of the last field set, which contains the correct
    //! uniform value.
    std::string last_field_set_;

    //! Creates and returns an Ion Uniform using the given registry.
    ion::gfx::Uniform CreateIonUniform_(
        const ion::gfx::ShaderInputRegistry &reg) const;

    //! Creates and returns an Ion array Uniform using the given registry.
    ion::gfx::Uniform CreateIonArrayUniform_(
        const ion::gfx::ShaderInputRegistry &reg) const;
};

}  // namespace SG
