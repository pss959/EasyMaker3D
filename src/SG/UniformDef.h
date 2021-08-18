#pragma once

#include <ion/gfx/uniform.h>

#include "NParser/ObjectSpec.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace SG {

//! A UniformDef object represents the definition of a Uniform that is
//! associated with a ShaderProgram.
class UniformDef : public Object {
  public:
    typedef ion::gfx::Uniform::ValueType ValueType;

    // XXXX
    ValueType GetValueType() const { return value_type_; }

    static NParser::ObjectSpec GetObjectSpec();

  private:
    // Parsed fields.
    ValueType value_type_ = ValueType::kFloatUniform;
};

}  // namespace SG
