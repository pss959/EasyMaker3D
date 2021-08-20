#include "Util/Search.h"

using ion::gfx::NodePtr;

namespace Util {

// XXXX Replace this with SG stuff.

static bool SearchForNode_(NodePath &cur_path, const std::string &name) {
    const NodePtr &cur_node = cur_path.back();
    if (cur_node->GetLabel() == name)
        return true;
    for (const auto &kid: cur_node->GetChildren()) {
        cur_path.push_back(kid);
        if (SearchForNode_(cur_path, name))
            return true;
        cur_path.pop_back();
    }
    return false;
}

bool SearchForNode(const ion::gfx::NodePtr &root,
                   const std::string &name, NodePath &path) {
    NodePath cur_path(1, root);
    return SearchForNode_(cur_path, name);
}

}  // namespace Util
