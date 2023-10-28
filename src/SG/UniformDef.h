#pragma once

#include <ion/gfx/uniform.h>

#include "SG/Object.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(UniformDef);

/// A UniformDef object represents the definition of a Uniform that is
/// associated with a ShaderProgram.
///
/// \ingroup SG
class UniformDef : public Object {
  public:
    using ValueType = ion::gfx::Uniform::ValueType;

    virtual bool IsNameRequired() const override { return true; }

    ValueType GetValueType() const { return value_type_; }

  protected:
    UniformDef() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<ValueType> value_type_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
