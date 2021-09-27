#pragma once

#include <ion/gfx/uniform.h>

#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace Parser { class Registry; }

namespace SG {

/// A UniformDef object represents the definition of a Uniform that is
/// associated with a ShaderProgram.
class UniformDef : public Object {
  public:
    typedef ion::gfx::Uniform::ValueType ValueType;

    virtual bool IsNameRequired() const override { return true; }

    virtual void AddFields() override;

    ValueType GetValueType() const { return value_type_; }

  protected:
    UniformDef() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<ValueType> value_type_{
        "value_type", ValueType::kFloatUniform};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
