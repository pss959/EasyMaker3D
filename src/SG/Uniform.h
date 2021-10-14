#pragma once

#include <vector>

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/uniform.h>
#include <ion/gfx/uniformblock.h>

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

    /// Sets the name of the field that will be used to create the Ion value.
    /// This should be called only if no value has been set; it asserts if that
    /// is not the case.
    void SetFieldName(const std::string &name);

    /// Creates a corresponding Ion Uniform using the given Ion registry and
    /// adds it to the given UniformBlock. Returns the resulting index.
    size_t SetUpIon(const ion::gfx::ShaderInputRegistry &reg,
                    ion::gfx::UniformBlock &block);

    /// Returns the index of the Ion Uniform within the Ion UniformBlock it was
    /// added to. This will be ion::base::kInvalidIndex until SetUpIon() is
    /// called.
    size_t GetIonIndex() const { return ion_index_; }

    /// Sets a single value in the Uniform based on the type of the value. This
    /// asserts that either no value has already been set or the value is of
    /// the same type as the existing one. Note that this <em>does not</em>
    /// modify any Ion uniform, since they are owned by the UniformBlock.
    template <typename T> void SetValue(const T &value);

  protected:
    Uniform() {}

    /// Redefines this to also copy last_field_set_.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

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

    /// Ion Uniform index. This is invalid until the Uniform is added to an Ion
    /// UniformBlock
    size_t ion_index_ = ion::base::kInvalidIndex;

    /// Creates and returns a single-valued Ion Uniform using the given
    /// registry.
    ion::gfx::Uniform CreateIonUniform_(
        const ion::gfx::ShaderInputRegistry &reg) const;

    /// Creates and returns an Ion array Uniform using the given registry.
    ion::gfx::Uniform CreateIonArrayUniform_(
        const ion::gfx::ShaderInputRegistry &reg) const;

    friend class Parser::Registry;
};

}  // namespace SG
