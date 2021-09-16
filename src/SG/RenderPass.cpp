#include "SG/RenderPass.h"

namespace SG {

void RenderPass::AddFields() {
    AddField(shader_programs_);
    AddField(root_node_);
}

ShaderProgramPtr RenderPass::FindShaderProgram(const std::string &name) const {
    // No name? Just use the first shader.
    if (name.empty())
        return GetShaderPrograms()[0];
    for (auto &program: GetShaderPrograms()) {
        if (program->GetName() == name)
            return program;
    }
    return ShaderProgramPtr();  // Not found.
}

ShaderProgramPtr RenderPass::GetDefaultShaderProgram() const {
    return FindShaderProgram(GetRootNode()->GetDefaultShaderName());
}

}  // namespace SG
