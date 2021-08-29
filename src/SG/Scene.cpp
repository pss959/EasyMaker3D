#include "SG/Scene.h"

#include "SG/SpecBuilder.h"
#include "SG/Camera.h"
#include "SG/Node.h"
#include "SG/PointLight.h"
#include "SG/RenderPass.h"
#include "SG/Visitor.h"

namespace SG {

NodePtr Scene::GetRootNode() const {
    NodePtr root;
    if (! render_passes_.empty())
        root = render_passes_.back()->GetRootNode();
    return root;
}

void Scene::Update() const {
    Visitor visitor;

    auto func = [](const NodePath &path){
        path.back()->Update();
        return Visitor::TraversalCode::kContinue;
    };

    for (const auto &pass: render_passes_)
        visitor.Visit(pass->GetRootNode(), func);
}

void Scene::SetUpIon(IonContext &context) {
    if (camera_)
        camera_->SetUpIon(context);
    for (const auto &light: lights_)
        light->SetUpIon(context);
    for (const auto &pass: render_passes_)
        pass->SetUpIon(context);
    Update();
}

Parser::ObjectSpec Scene::GetObjectSpec() {
    SG::SpecBuilder<Scene> builder;
    builder.AddObject<Camera>("camera",                &Scene::camera_);
    builder.AddObjectList<PointLight>("lights",        &Scene::lights_);
    builder.AddObjectList<RenderPass>("render_passes", &Scene::render_passes_);
    return Parser::ObjectSpec{
        "Scene", false, []{ return new Scene; }, builder.GetSpecs() };
}

}  // namespace SG
