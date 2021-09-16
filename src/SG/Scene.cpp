#include "SG/Scene.h"

#include "SG/Node.h"

namespace SG {

void Scene::AddFields() {
    AddField(gantry_);
    AddField(lights_);
    AddField(render_passes_);
}

NodePtr Scene::GetRootNode() const {
    NodePtr root;
    if (! GetRenderPasses().empty())
        root = GetRenderPasses().back()->GetRootNode();
    return root;
}

}  // namespace SG
