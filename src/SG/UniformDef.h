#pragma once

#include <ion/gfx/uniform.h>

#include "Base/Memory.h"
#include "SG/Object.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(UniformDef);

/// A UniformDef object represents the definition of a Uniform that is
/// associated with a ShaderProgram.
///
/// \ingroup SG
class UniformDef : public Object {
  public:
    typedef ion::gfx::Uniform::ValueType ValueType;

    virtual bool IsNameRequired() const override { return true; }

    ValueType GetValueType() const { return value_type_; }

  protected:
    UniformDef() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<ValueType> value_type_{
        "value_type", ValueType::kFloatUniform};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
