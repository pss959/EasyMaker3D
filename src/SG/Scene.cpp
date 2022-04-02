#include "SG/Scene.h"

#include "Managers/ColorManager.h"
#include "SG/Node.h"
#include "SG/ShaderProgram.h"
#include "Util/KLog.h"

namespace SG {

void Scene::AddFields() {
    AddField(log_key_string_);
    AddField(colors_);
    AddField(gantry_);
    AddField(lights_);
    AddField(render_passes_);
    AddField(root_node_);
    Object::AddFields();
}

bool Scene::IsValid(std::string &details) {
    if (! Object::IsValid(details))
        return false;

    // Make sure the colors UniformBlock has all correct uniforms.
    if (GetColors()) {
        for (const auto &u: GetColors()->GetUniforms()) {
            if (u->GetLastFieldSet() != "vec4f_val") {
                details = "Color " + u->GetDesc() +
                    " has missing or wrong value type";
                return false;
            }
        }
    }

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
    else if (&field == &colors_) {
        if (GetColors()) {
            for (auto &u: GetColors()->GetUniforms())
                ColorManager::AddSpecialColor(u->GetName(),
                                              Color(u->GetVector4f()));
        }
    }
}

void Scene::SetUpIon(const IonContextPtr &ion_context) {
    // First set up all Ion ShaderPrograms in all render passes.
    for (const auto &pass: GetRenderPasses()) {
        pass->SetUpIon(ion_context->GetTracker(),
                       *ion_context->GetShaderManager());
    }

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
}

}  // namespace SG
