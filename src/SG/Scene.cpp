#include "SG/Scene.h"

#include "SG/Node.h"
#include "Util/KLog.h"

namespace SG {

void Scene::AddFields() {
    AddField(log_key_string_);
    AddField(gantry_);
    AddField(lights_);
    AddField(render_passes_);
}

void Scene::SetFieldParsed(const Parser::Field &field) {
    if (&field == &log_key_string_)
        KLogger::SetKeyString(log_key_string_);
}

NodePtr Scene::GetRootNode() const {
    NodePtr root;
    if (! GetRenderPasses().empty())
        root = GetRenderPasses().back()->GetRootNode();
    return root;
}

}  // namespace SG
