#pragma once

#include <vector>

#include <ion/gfx/shaderprogram.h>

#include "SG/Object.h"
#include "SG/ShaderSource.h"
#include "SG/Typedefs.h"
#include "SG/UniformDef.h"

namespace SG {

//! A ShaderProgram object wraps an Ion ShaderProgram.
class ShaderProgram : public Object {
  public:
    virtual bool IsNameRequired() const override { return true; }

    virtual void AddFields() override;

    //! Returns the associated Ion ShaderProgram.
    const ion::gfx::ShaderProgramPtr &GetIonShaderProgram() {
        return ion_program_;
    }

    const std::vector<UniformDefPtr> & GetUniformDefs() const {
        return uniform_defs_;
    }
    bool            ShouldInheritUniforms() const { return inherit_uniforms_; }
    ShaderSourcePtr GetVertexSource()       const { return vertex_source_;    }
    ShaderSourcePtr GetGeometrySource()     const { return geometry_source_;  }
    ShaderSourcePtr GetFragmentSource()     const { return fragment_source_;  }

    virtual void SetUpIon(IonContext &context) override;

  private:
    ion::gfx::ShaderProgramPtr ion_program_;  //! Associated Ion ShaderProgram.

    //! \name Parsed Fields
    //!@{
    Parser::TField<bool>                inherit_uniforms_{"inherit_uniforms", false};
    Parser::ObjectListField<UniformDef> uniform_defs_{"uniform_defs"};
    Parser::ObjectField<ShaderSource>   vertex_source_{"vertex_source"};
    Parser::ObjectField<ShaderSource>   geometry_source_{"geometry_source"};
    Parser::ObjectField<ShaderSource>   fragment_source_{"fragment_source"};
    //!@}
};

}  // namespace SG
