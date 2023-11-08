#include "SG/Scene.h"

#include "SG/IonContext.h"
#include "SG/Node.h"
#include "SG/ShaderProgram.h"
#include "Util/KLog.h"

namespace SG {

void Scene::AddFields() {
    AddField(log_key_string_.Init("log_key_string"));
    AddField(color_map_.Init("color_map"));
    AddField(gantry_.Init("gantry"));
    AddField(lights_.Init("lights"));
    AddField(render_passes_.Init("render_passes"));
    AddField(root_node_.Init("root_node"));

    Object::AddFields();
}

bool Scene::IsValid(Str &details) {
    if (! Object::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]

    // Make sure each RenderPass has at least one shader program.
    for (const auto &pass: GetRenderPasses()) {
        if (pass->GetShaderPrograms().empty()) {
            details = "No shader programs for " + pass->GetDesc();
            return false;
        }
    }

    return true;
}

void Scene::SetFieldParsed(const Parser::Field &field) {
    if (&field == &log_key_string_) {
        KLogger::AppendKeyString(log_key_string_);
    }
}

void Scene::SetUpIon(const IonContextPtr &ion_context) {
    KLOG('Z', ion_context->GetIndent() << "SetUpIon for " << GetDesc());
    ion_context->ChangeLevel(1);

    // First set up all Ion ShaderPrograms in all render passes.
    for (const auto &pass: GetRenderPasses())
        pass->SetUpIon(ion_context);

    // Add the pass names to the IonContext.
    for (const auto &pass: GetRenderPasses()) {
        ASSERT(! pass->GetName().empty());
        ion_context->AddPassName(pass->GetName());
    }

    // Add the Ion ShaderProgram for each SG::ShaderProgram in each RenderPass.
    for (const auto &pass: GetRenderPasses()) {
        for (const auto &program: pass->GetShaderPrograms())
            ion_context->AddShaderProgram(program->GetName(), pass->GetName(),
                                          program->GetIonShaderProgram());
    }

    // Set up the programs vector to pass to nodes. Start with the default
    // ShaderProgram for each pass.
    std::vector<ion::gfx::ShaderProgramPtr> programs;
    for (const auto &pass: GetRenderPasses()) {
        programs.push_back(
            pass->GetDefaultShaderProgram()->GetIonShaderProgram());
    }

    // Now set up the root node.
    if (const NodePtr &root = GetRootNode())
        root->SetUpIon(ion_context, programs);

    ion_context->ChangeLevel(-1);
}

}  // namespace SG
