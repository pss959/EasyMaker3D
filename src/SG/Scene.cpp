#include "SG/Scene.h"

#include "SG/SpecBuilder.h"
#include "SG/Camera.h"
#include "SG/Node.h"

namespace SG {

void Scene::SetUpIon(IonContext &context) {
    if (camera_)
        camera_->SetUpIon(context);
    if (root_)
        root_->SetUpIon(context);
}

Parser::ObjectSpec Scene::GetObjectSpec() {
    SG::SpecBuilder<Scene> builder;
    builder.AddObject<Camera>("camera", &Scene::camera_);
    builder.AddObject<Node>("root",     &Scene::root_);
    return Parser::ObjectSpec{
        "Scene", false, []{ return new Scene; }, builder.GetSpecs() };
}

}  // namespace SG
