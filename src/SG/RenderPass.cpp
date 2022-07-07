#include "SG/RenderPass.h"

#include "SG/RenderData.h"

namespace SG {

void RenderPass::AddFields() {
    AddField(shader_programs_);
    Object::AddFields();
}

void RenderPass::SetUpIon(FileMap &file_map,
                          ion::gfxutils::ShaderManager &shader_manager) {
    for (auto &program: GetShaderPrograms())
        program->SetUpIon(file_map, shader_manager);
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
