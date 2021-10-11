#include "SG/RenderPass.h"

namespace SG {

void RenderPass::AddFields() {
    AddField(shader_programs_);
}

void RenderPass::SetUpIon(Tracker &tracker,
                          ion::gfxutils::ShaderManager &shader_manager) {
    for (auto &program: GetShaderPrograms())
        program->SetUpIon(tracker, shader_manager);
}

ShaderProgramPtr RenderPass::GetDefaultShaderProgram() const {
    ASSERT(! GetShaderPrograms().empty());
    return GetShaderPrograms()[0];
}

ShaderProgramPtr RenderPass::FindShaderProgram(const std::string &name) const {
    ASSERT(! name.empty());
    for (auto &program: GetShaderPrograms()) {
        if (program->GetName() == name)
            return program;
    }
    return ShaderProgramPtr();  // Not found.
}

}  // namespace SG
