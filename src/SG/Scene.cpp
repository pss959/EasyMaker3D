#include "SG/Scene.h"

#include "SG/SpecBuilder.h"
#include "SG/Camera.h"
#include "SG/Node.h"

namespace SG {

void Scene::Finalize() {
    if (! camera_)
        camera_.reset(new Camera());
    if (! root_)
        root_.reset(new Node());
}

NParser::ObjectSpec Scene::GetObjectSpec() {
    SG::SpecBuilder<Scene> builder(Resource::GetObjectSpec().field_specs);
    builder.AddObject<Camera>("camera", &Scene::camera_);
    builder.AddObject<Node>("root",     &Scene::root_);
    return NParser::ObjectSpec{
        "Scene", false, []{ return new Scene; }, builder.GetSpecs() };
}

}  // namespace SG
