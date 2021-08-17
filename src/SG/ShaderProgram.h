#pragma once

#include <ion/gfx/shaderprogram.h>

#include "NParser/FieldSpec.h"
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

    // XXXX
    ShaderSourcePtr GetVertexSource()   const { return vertex_source_;   }
    ShaderSourcePtr GetGeometrySource() const { return geometry_source_; }
    ShaderSourcePtr GetFragmentSource() const { return fragment_source_; }

    static std::vector<NParser::FieldSpec> GetFieldSpecs();

  private:
    ion::gfx::ShaderProgramPtr ion_program_;  //! Associated Ion ShaderProgram.

    // Parsed fields.
    ShaderSourcePtr vertex_source_;
    ShaderSourcePtr geometry_source_;
    ShaderSourcePtr fragment_source_;

    //! Redefines this to set up the Ion ShaderProgram.
    virtual void Finalize() override;
};

}  // namespace SG
