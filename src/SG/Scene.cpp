#include "SG/Scene.h"

#include "SG/Node.h"
#include "SG/Visitor.h"

namespace SG {

// ----------------------------------------------------------------------------
// Scene::Updater_ class.
// ----------------------------------------------------------------------------

//! Derived Visitor class that is used to update all Nodes in the graph.
class Scene::Updater_ : public Visitor {
  protected:
    virtual TraversalCode VisitNodeStart(const NodePath &path) override {
        path.back()->Update();
        return Visitor::TraversalCode::kContinue;
    }
};

// ----------------------------------------------------------------------------
// Scene functions.
// ----------------------------------------------------------------------------

void Scene::AddFields() {
    AddField(camera_);
    AddField(lights_);
    AddField(render_passes_);
}

NodePtr Scene::GetRootNode() const {
    NodePtr root;
    if (! GetRenderPasses().empty())
        root = GetRenderPasses().back()->GetRootNode();
    return root;
}

void Scene::Update() const {
    Updater_ updater;
    for (const auto &pass: GetRenderPasses())
        updater.Visit(pass->GetRootNode());
}

void Scene::SetUpIon(const ContextPtr &context) {
    Object::SetUpIon(context);

    context->pass_type = PassType::kAnyPass;
    if (GetCamera())
        GetCamera()->SetUpIon(context);
    for (const auto &light: GetLights())
        light->SetUpIon(context);
    for (const auto &pass: GetRenderPasses()) {
        context->pass_type = pass->GetPassType();
        pass->SetUpIon(context);
    }
    Update();
}

}  // namespace SG
