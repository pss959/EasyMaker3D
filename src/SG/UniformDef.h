#pragma once

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/uniform.h>

#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace SG {

//! A UniformDef object represents the definition of a Uniform that is
//! associated with a ShaderProgram.
class UniformDef : public Object {
  public:
    typedef ion::gfx::Uniform::ValueType ValueType;

    virtual bool IsNameRequired() const override { return true; }

    virtual void AddFields() override;

    //! Returns an Ion ShaderInputRegistry::UniformSpec representing the
    //! Uniform definition.
    const ion::gfx::ShaderInputRegistry::UniformSpec & GetIonSpec() {
        return spec_;
    }

    ValueType GetValueType() const { return value_type_; }

    virtual void SetUpIon(IonContext &context) override;

  private:
    ion::gfx::ShaderInputRegistry::UniformSpec spec_;

    //! \name Parsed Fields
    //!@{
    Parser::EnumField<ValueType> value_type_{
        "value_type", ValueType::kFloatUniform};
    //!@}
};

}  // namespace SG
