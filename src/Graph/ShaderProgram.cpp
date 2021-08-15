#include "Graph/ShaderProgram.h"

#include "Graph/ShaderSource.h"

namespace Graph {

void ShaderProgram::SetIonShaderProgram_(
    const ion::gfx::ShaderProgramPtr &program) {
    i_program_ = program;
}

void ShaderProgram::SetVertexSource_(const ShaderSourcePtr &source) {
    vertex_source_ = source;
}

void ShaderProgram::SetGeometrySource_(const ShaderSourcePtr &source) {
    geometry_source_ = source;
}

void ShaderProgram::SetFragmentSource_(const ShaderSourcePtr &source) {
    fragment_source_ = source;
}

}  // namespace Graph
