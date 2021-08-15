#pragma once

#include <vector>

#include <ion/gfx/shaderprogram.h>

#include "Graph/Typedefs.h"

namespace Input { class Extractor; }

namespace Graph {

//! A ShaderProgram object encapsulates an Ion ShaderProgram.
class ShaderProgram {
  public:
    //! Returns the associated Ion ShaderProgram.
    const ion::gfx::ShaderProgramPtr &GetIonShaderProgram() {
        return i_program_;
    }

    // XXXX
    ShaderSourcePtr GetVertexSource()   const { return vertex_source_;   }
    ShaderSourcePtr GetGeometrySource() const { return geometry_source_; }
    ShaderSourcePtr GetFragmentSource() const { return fragment_source_; }

  private:
    ion::gfx::ShaderProgramPtr  i_program_;  //! Associated Ion ShaderProgram.

    ShaderSourcePtr vertex_source_;
    ShaderSourcePtr geometry_source_;
    ShaderSourcePtr fragment_source_;

    // XXXX
    void SetIonShaderProgram_(const ion::gfx::ShaderProgramPtr &program);
    void SetVertexSource_(const ShaderSourcePtr &source);
    void SetGeometrySource_(const ShaderSourcePtr &source);
    void SetFragmentSource_(const ShaderSourcePtr &source);

    friend class ::Input::Extractor;
};

}  // namespace Graph
