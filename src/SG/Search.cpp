#include "SG/Search.h"

#include "SG/Node.h"
#include "SG/Scene.h"

namespace SG {

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

/// This recursive function does most of the work for path searching.
static bool SearchPath_(NodePath &cur_path, const std::string &name) {
    const NodePtr &cur_node = cur_path.back();
    if (cur_node->GetName() == name)
        return true;
    for (const auto &kid: cur_node->GetChildren()) {
        cur_path.push_back(kid);
        if (SearchPath_(cur_path, name))
            return true;
        cur_path.pop_back();
    }
    return false;
}

/// This recursive function does most of the work for non-path searching.
static NodePtr SearchUnderNode_(const Node &cur_node, const std::string &name) {
    NodePtr found;
    for (const auto &kid: cur_node.GetChildren()) {
        found = kid->GetName() == name ? kid : SearchUnderNode_(*kid, name);
        if (found)
            break;
    }
    return found;
}

/// Recursive function that does the work for FindNodes().
static void FindNodesUnder_(const NodePtr &root,
                            const std::function<bool(const Node &)> &func,
                            std::vector<NodePtr> &found_nodes) {
    if (func(*root))
        found_nodes.push_back(root);
    for (const auto &kid: root->GetChildren())
        FindNodesUnder_(kid, func, found_nodes);
}

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

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
    if (! SearchPath_(cur_path, name)) {
        cur_path.clear();
        if (! ok_if_not_found)
            ASSERTM(false, "Node '" + name + "' not found under node " +
                    root->GetDesc());
    }
    return cur_path;
}

NodePtr FindNodeUnderNode(const Node &root, const std::string &name,
                          bool ok_if_not_found) {
    NodePtr found = SearchUnderNode_(root, name);
    if (found)
        return found;
    if (! ok_if_not_found)
        ASSERTM(false, "Node '" + name + "' not found under node " +
                root.GetDesc());
    return NodePtr();
}

std::vector<NodePtr> FindNodes(const NodePtr &root,
                               const std::function<bool(const Node &)> &func) {
    ASSERT(root);
    ASSERT(func);
    std::vector<NodePtr> nodes;
    FindNodesUnder_(root, func, nodes);

    // Sort and uniquify the vector.
    std::sort(nodes.begin(), nodes.end());
    nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());

    return nodes;
}

}  // namespace SG
