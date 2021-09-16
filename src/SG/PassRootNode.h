#pragma once

#include <string>

#include "SG/Node.h"

namespace SG {

//! PassRootNode is a derived Node that is used as the root node in a
//! RenderPass. It adds a field indicating what the default shader is for the
//! pass and is set up differently.
class PassRootNode : public Node {
  public:
    virtual void AddFields() override;

    const std::string & GetDefaultShaderName() const {
        return default_shader_name_;
    }

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<std::string>  default_shader_name_{"default_shader_name"};
    //!@}
};

}  // namespace SG
