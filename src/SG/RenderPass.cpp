#include "SG/RenderPass.h"

#include <algorithm>

#include "SG/Node.h"
#include "SG/RenderData.h"
#include "SG/Search.h"

namespace SG {

void RenderPass::AddFields() {
    AddField(shader_programs_.Init("shader_programs"));

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

std::vector<NodePtr> RenderPass::FindNodesMatchingShaderName(
    const NodePtr &root, const std::string &prefix) {
    auto match = [&](const SG::Node &node){
        const auto &names = node.GetShaderNames();
        const auto find =
            [&](const std::string &name){ return name.starts_with(prefix); };
        return std::find_if(names.begin(), names.end(), find) != names.end();
    };
    return SG::FindUniqueNodes(root, match);
}

}  // namespace SG
