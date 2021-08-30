#include "SG/Search.h"

#include "SG/Node.h"
#include "SG/Scene.h"

namespace SG {

//! This recursive function does most of the work.
static bool Search_(NodePath &cur_path, const std::string &name) {
    const NodePtr &cur_node = cur_path.back();
    if (cur_node->GetName() == name)
        return true;
    for (const auto &kid: cur_node->GetChildren()) {
        cur_path.push_back(kid);
        if (Search_(cur_path, name))
            return true;
        cur_path.pop_back();
    }
    return false;
}

NodePath FindNodePathInScene(const Scene &scene, const std::string &name,
                             bool ok_if_not_found) {
    if (scene.GetRootNode())
        return FindNodePathUnderNode(scene.GetRootNode(), name,
                                     ok_if_not_found);
    if (! ok_if_not_found)
        ASSERTM(false, "Node '" + name + "' not found in scene");
    return NodePath();
}

NodePtr FindNodeInScene(const Scene &scene, const std::string &name,
                        bool ok_if_not_found) {
    NodePath path = FindNodePathInScene(scene, name, ok_if_not_found);
    if (! path.empty())
        return path.back();
    return NodePtr();
}

NodePath FindNodePathUnderNode(const NodePtr &root, const std::string &name,
                               bool ok_if_not_found) {
    NodePath cur_path(root);
    if (! Search_(cur_path, name)) {
        cur_path.clear();
        if (! ok_if_not_found)
            ASSERTM(false, "Node '" + name + "' not found under node");
    }
    return cur_path;
}

NodePtr FindNodeUnderNode(const NodePtr &root, const std::string &name,
                          bool ok_if_not_found) {
    NodePath cur_path(root);
    if (Search_(cur_path, name))
        return cur_path.back();

    if (! ok_if_not_found)
        ASSERTM(false, "Node '" + name + "' not found under node");
    return NodePtr();
}

}  // namespace SG
