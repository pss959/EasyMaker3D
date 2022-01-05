#include "SG/Search.h"

#include "SG/Node.h"
#include "SG/Scene.h"

namespace SG {

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

/// This recursive function does most of the work for path searching by pointer.
static bool SearchPath_(NodePath &cur_path, const NodePtr &node) {
    const NodePtr &cur_node = cur_path.back();
    if (! cur_node->IsEnabled(Node::Flag::kSearch))
        return false;
    if (cur_node == node)
        return true;
    for (const auto &kid: cur_node->GetAllChildren()) {
        cur_path.push_back(kid);
        if (SearchPath_(cur_path, node))
            return true;
        cur_path.pop_back();
    }
    return false;
}

/// This recursive function does most of the work for path searching by name.
static bool SearchPath_(NodePath &cur_path, const std::string &name) {
    const NodePtr &cur_node = cur_path.back();
    if (! cur_node->IsEnabled(Node::Flag::kSearch))
        return false;
    if (cur_node->GetName() == name)
        return true;
    for (const auto &kid: cur_node->GetAllChildren()) {
        cur_path.push_back(kid);
        if (SearchPath_(cur_path, name))
            return true;
        cur_path.pop_back();
    }
    return false;
}

/// This recursive function does most of the work for non-path searching by
/// name.
static NodePtr SearchNameUnderNode_(const Node &cur_node,
                                    const std::string &name) {
    NodePtr found;
    if (cur_node.IsEnabled(Node::Flag::kSearch)) {
        for (const auto &kid: cur_node.GetAllChildren()) {
            found = kid->GetName() == name ? kid :
                SearchNameUnderNode_(*kid, name);
            if (found)
                break;
        }
    }
    return found;
}

/// This recursive function does most of the work for non-path searching by
/// type name.
static NodePtr SearchTypeUnderNode_(const Node &cur_node,
                                    const std::string &type_name) {
    NodePtr found;
    if (cur_node.IsEnabled(Node::Flag::kSearch)) {
        for (const auto &kid: cur_node.GetAllChildren()) {
            found = kid->GetTypeName() == type_name ? kid :
                SearchTypeUnderNode_(*kid, type_name);
            if (found)
                break;
        }
    }
    return found;
}

/// Recursive function that does the work for FindNodes().
static void FindNodesUnder_(const NodePtr &root,
                            const std::function<bool(const Node &)> &func,
                            std::vector<NodePtr> &found_nodes) {
    if (root->IsEnabled(Node::Flag::kSearch)) {
        if (func(*root))
            found_nodes.push_back(root);
        for (const auto &kid: root->GetAllChildren())
            FindNodesUnder_(kid, func, found_nodes);
    }
}

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

NodePath FindNodePathInScene(const Scene &scene, const NodePtr &node) {
    if (scene.GetRootNode())
        return FindNodePathUnderNode(scene.GetRootNode(), node);
    return NodePath();
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

NodePath FindNodePathUnderNode(const NodePtr &root, const NodePtr &node) {
    NodePath cur_path(root);
    if (! SearchPath_(cur_path, node))
        cur_path.clear();
    return cur_path;
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
    NodePtr found = SearchNameUnderNode_(root, name);
    if (found)
        return found;
    if (! ok_if_not_found)
        ASSERTM(false, "Node '" + name + "' not found under node " +
                root.GetDesc());
    return NodePtr();
}

NodePtr FindFirstTypedNodeUnderNode(const Node &root,
                                    const std::string &type_name) {
    return SearchTypeUnderNode_(root, type_name);
}

std::vector<NodePtr> FindNodes(const NodePtr &root,
                               const std::function<bool(const Node &)> &func) {
    ASSERT(root);
    ASSERT(func);
    std::vector<NodePtr> nodes;
    FindNodesUnder_(root, func, nodes);
    return nodes;
}

std::vector<NodePtr> FindUniqueNodes(
    const NodePtr &root, const std::function<bool(const Node &)> &func) {
    std::vector<NodePtr> nodes = FindNodes(root, func);

    // Sort and uniquify the vector.
    std::sort(nodes.begin(), nodes.end(),
              [](const NodePtr &n0,
                 const NodePtr &n1){ return n0->GetName() < n1->GetName(); });
    nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());

    return nodes;
}

}  // namespace SG
