#pragma once

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/uniform.h>

#include "Math/Types.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace Parser { class Registry; }

namespace SG {

/// A Uniform object wraps an Ion uniform.
class Uniform : public Object {
  public:
    virtual bool IsNameRequired() const override { return true; }

    virtual void AddFields() override;

    /// Redefines this to save the name of the last field parsed, which is the
    /// current value of the uniform.
    virtual void SetFieldParsed(const Parser::Field &field) override {
        last_field_set_ = field.GetName();
    }

    /// Creates and returns an Ion Uniform using the given registry.
    ion::gfx::Uniform CreateIonUniform(
        const ion::gfx::ShaderInputRegistry &reg) const;

    /// Returns the count of values. A count greater than 1 creates an array
    /// uniform.
    int GetCount() const { return count_; }

    /// Returns the name of the last field whose value was set. This will be
    /// empty if none was set.
    const std::string & GetLastFieldSet() const { return last_field_set_; }

    /// \name Value Access
    /// Only one of these will return the actual value of the uniform,
    /// depending on what GetLastFieldSet() returns.
    ///@{
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
    ///@}

  protected:
    Uniform() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<int>          count_{"count", 1};
    Parser::TField<float>        float_val_{"float_val"};
    Parser::TField<int>          int_val_{"int_val"};
    Parser::TField<unsigned int> uint_val_{"uint_val"};
    Parser::TField<Vector2f>     vec2f_val_{"vec2f_val"};
    Parser::TField<Vector3f>     vec3f_val_{"vec3f_val"};
    Parser::TField<Vector4f>     vec4f_val_{"vec4f_val"};
    Parser::TField<Vector2i>     vec2i_val_{"vec2i_val"};
    Parser::TField<Vector3i>     vec3i_val_{"vec3i_val"};
    Parser::TField<Vector4i>     vec4i_val_{"vec4i_val"};
    Parser::TField<Vector2ui>    vec2ui_val_{"vec2ui_val"};
    Parser::TField<Vector3ui>    vec3ui_val_{"vec3ui_val"};
    Parser::TField<Vector4ui>    vec4ui_val_{"vec4ui_val"};
    Parser::TField<Matrix2f>     mat2_val_{"mat2_val"};
    Parser::TField<Matrix3f>     mat3_val_{"mat3_val"};
    Parser::TField<Matrix4f>     mat4_val_{"mat4_val"};
    ///@}

    /// Stores the name of the last field set, which contains the correct
    /// uniform value.
    std::string last_field_set_;

    /// Creates and returns a single-valued Ion Uniform using the given
    /// registry.
    ion::gfx::Uniform CreateIonUniform_(
        const ion::gfx::ShaderInputRegistry &reg) const;

    /// Creates and returns an Ion array Uniform using the given registry.
    ion::gfx::Uniform CreateIonArrayUniform_(
        const ion::gfx::ShaderInputRegistry &reg) const;

    /// Adds an Ion Uniform to the Ion UniformBlock, asserting that the
    /// addition succeeded. Returns the resulting index.
    size_t AddIonUniform_(const ion::gfx::Uniform &uniform);

    friend class Parser::Registry;
};

}  // namespace SG
