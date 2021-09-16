#pragma once

#include "SG/Object.h"
#include "SG/UniformBlock.h"
#include "SG/Typedefs.h"

namespace SG {

//! A PassData object stores a UniformBlock for a Node that may be applied for
//! a specific render pass. The name of the PassData indicates which pass this
//! is applicable to; if the name is empty (the default), it applies to all
//! render passes.  It may also specify the name of a shader to use for the
//! Node for the render pass. If the name is empty (the default), the current
//! shader is used.
//!
//! Note that the PassData may contain a null UniformBlock pointer.
class PassData : public Object {
  public:
    //! Default constructor.
    PassData() {}

    //! Constructs an instance with the given name and UniformBlock.
    PassData(const std::string &name, const UniformBlockPtr &uniform_block) {
        SetName(name);
        uniform_block_ = uniform_block;
    }

    const std::string     & GetShaderName() const   { return shader_name_; }
    const UniformBlockPtr & GetUniformBlock() const { return uniform_block_; }

    virtual void AddFields() override;

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<std::string>       shader_name_{"shader_name"};
    Parser::ObjectField<UniformBlock> uniform_block_{"uniforms"};
    //!@}
};

}  // namespace SG
