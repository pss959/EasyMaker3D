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


std::vector<NParser::FieldSpec> Scene::GetFieldSpecs_() {
    SG::SpecBuilder<Scene> builder;
    builder.AddObject<Camera>("camera", &Scene::camera_);
    builder.AddObject<Node>("root",     &Scene::root_);
    return builder.GetSpecs();
}

}  // namespace SG
