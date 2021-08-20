#pragma once

#include <vector>

#include <ion/gfx/shaderprogram.h>

#include "Parser/ObjectSpec.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace SG {

//! A ShaderProgram object wraps an Ion ShaderProgram.
class ShaderProgram : public Object {
  public:
    //! Returns the associated Ion ShaderProgram.
    const ion::gfx::ShaderProgramPtr &GetIonShaderProgram() {
        return ion_program_;
    }

    const std::vector<UniformDefPtr> & GetUniformDefs() const {
        return uniform_defs_;
    }
    ShaderSourcePtr GetVertexSource()   const { return vertex_source_;   }
    ShaderSourcePtr GetGeometrySource() const { return geometry_source_; }
    ShaderSourcePtr GetFragmentSource() const { return fragment_source_; }

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    ion::gfx::ShaderProgramPtr ion_program_;  //! Associated Ion ShaderProgram.

    //! \name Parsed Fields
    //!@{
    std::vector<UniformDefPtr> uniform_defs_;
    ShaderSourcePtr            vertex_source_;
    ShaderSourcePtr            geometry_source_;
    ShaderSourcePtr            fragment_source_;
    //!@}
};

}  // namespace SG
