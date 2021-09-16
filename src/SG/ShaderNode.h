#pragma once

#include <string>

#include "SG/Node.h"
#include "SG/ShaderProgram.h"

namespace SG {

//! ShaderNode is a derived Node that is used to switch shaders for a
//! particular pass during rendering. It is also used as the root node of a
//! RenderPass to set up the default shader for the pass.
class ShaderNode : public Node {
  public:
    virtual void AddFields() override;

    const std::string & GetPassName()   const { return pass_name_;   }
    const std::string & GetShaderName() const { return shader_name_; }

    //! Sets the ShaderProgram once it is known.
    void SetShaderProgram(const ShaderProgramPtr &program) {
        shader_program_ = program;
    }

    //! Returns the ShaderProgram. This will be null until SetShaderProgram()
    //! is called.
    const ShaderProgramPtr & GetShaderProgram() const {
        return shader_program_;
    }

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<std::string>  pass_name_{"pass_name"};
    Parser::TField<std::string>  shader_name_{"shader_name"};
    //!@}

    //! ShaderProgram selected by the ShaderNode, once it is known.
    ShaderProgramPtr shader_program_;
};

}  // namespace SG
