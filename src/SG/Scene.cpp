#include "SG/Scene.h"

#include "SG/SpecBuilder.h"
#include "SG/Camera.h"
#include "SG/Node.h"
#include "SG/ShaderProgram.h"

namespace SG {

void Scene::SetUpIon(IonContext &context) {
    if (camera_)
        camera_->SetUpIon(context);
    if (shader_) {
        shader_->SetUpIon(context);
        // XXXX
        context.current_registry =
            shader_->GetIonShaderProgram()->GetRegistry();
    }
    if (shadow_shader_)
        shadow_shader_->SetUpIon(context);
    if (root_)
        root_->SetUpIon(context);

    context.current_registry =
        ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
}

Parser::ObjectSpec Scene::GetObjectSpec() {
    SG::SpecBuilder<Scene> builder;
    builder.AddObject<Camera>("camera",               &Scene::camera_);
    builder.AddObject<ShaderProgram>("shader",        &Scene::shader_);
    builder.AddObject<ShaderProgram>("shadow_shader", &Scene::shadow_shader_);
    builder.AddObject<Node>("root",                   &Scene::root_);
    return Parser::ObjectSpec{
        "Scene", false, []{ return new Scene; }, builder.GetSpecs() };
}

}  // namespace SG
