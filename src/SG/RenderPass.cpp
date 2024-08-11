//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/RenderPass.h"

#include <algorithm>

#include "SG/IonContext.h"
#include "SG/Node.h"
#include "SG/RenderData.h"
#include "SG/Search.h"
#include "Util/KLog.h"

namespace SG {

void RenderPass::AddFields() {
    AddField(shader_programs_.Init("shader_programs"));

    Object::AddFields();
}

void RenderPass::SetUpIon(const IonContextPtr &ion_context) {
    KLOG('Z', ion_context->GetIndent() << "SetUpIon for " << GetDesc());
    ion_context->ChangeLevel(1);
    for (auto &program: GetShaderPrograms())
        program->SetUpIon(ion_context);
    ion_context->ChangeLevel(-1);
}

ShaderProgramPtr RenderPass::GetDefaultShaderProgram() const {
    ASSERT(! GetShaderPrograms().empty());
    return GetShaderPrograms()[0];
}

std::vector<NodePtr> RenderPass::FindNodesMatchingShaderName(
    const NodePtr &root, const Str &prefix) {
    auto match = [&](const SG::Node &node){
        const auto &names = node.GetShaderNames();
        const auto find =
            [&](const Str &name){ return name.starts_with(prefix); };
        return std::find_if(names.begin(), names.end(), find) != names.end();
    };
    return SG::FindUniqueNodes(root, match);
}

}  // namespace SG
