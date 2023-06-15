#pragma once

#include <functional>
#include <string>
#include <vector>

#include "SG/Node.h"
#include "SG/NodePath.h"
#include "Util/Assert.h"

/// \file
/// \name Graph Search Utilities
/// This file contains utility functions used to find SG Nodes inside scenes.
/// Each of these takes a \c ok_if_not_found flag that defaults to false. An
/// assertion will be triggered if this is set to true and the searched-for
/// Node is not found.
///
/// \ingroup SG

namespace SG {

class Scene;

/// Typedef for a function that returns true if a Node matches a specific
/// predicate.
typedef std::function<bool(const Node &)> NodePredicate;

/// Finds the first path from the root node of the given scene to the given
/// node. Returns an empty NodePath if not found.
NodePath FindNodePathInScene(const Scene &scene, const Node &node);

/// Finds the named node in a scene, returning a path from the root node of the
/// scene to it. Returns an empty NodePath if not found.
NodePath FindNodePathInScene(const Scene &scene, const std::string &name,
                             bool ok_if_not_found = false);

/// Finds the named node in a scene, returning a pointer to it. Returns a null
/// pointer if not found.
NodePtr FindNodeInScene(const Scene &scene, const std::string &name,
                        bool ok_if_not_found = false);

/// Searches under the given root node for the given node, returning a path
/// from the root to the found node. Returns an empty NodePath if not found.
NodePath FindNodePathUnderNode(const NodePtr &root, const Node &node);

/// Searches under the given root node for a node with the given name,
/// returning a path from the root to the found node. Returns an empty NodePath
/// if not found.
NodePath FindNodePathUnderNode(const NodePtr &root, const std::string &name,
                               bool ok_if_not_found = false);

/// Searches under the given root node (exclusively) for a node with the given
/// name, returning a pointer to it. Returns a null pointer if not found.
NodePtr FindNodeUnderNode(const Node &root, const std::string &name,
                          bool ok_if_not_found = false);

/// Templated version of FindNodeInScene() that casts the returned NodePtr to
/// the templated type (derived from Node). This always asserts on failure.
template <typename T> std::shared_ptr<T> FindTypedNodeInScene(
    const Scene &scene, const std::string &name) {
    NodePtr node = FindNodeInScene(scene, name, false);
    std::shared_ptr<T> typed_node = std::dynamic_pointer_cast<T>(node);
    ASSERTM(typed_node, "Typed Node '" + name + "' not found in scene");
    return typed_node;
}

/// Templated version of FindNodeUnderNode() that casts the returned NodePtr to
/// the templated type (derived from Node). This always asserts on failure.
template <typename T> std::shared_ptr<T> FindTypedNodeUnderNode(
    const Node &root, const std::string &name) {
    std::shared_ptr<T> typed_node = std::dynamic_pointer_cast<T>(
        FindNodeUnderNode(root, name, false));
    ASSERTM(typed_node, "Typed Node '" + name + "' not found under " +
            root.GetDesc());
    return typed_node;
}

/// Searches under the given root node (exclusively) for the first node with
/// the given type name, returning a pointer to it. Returns a null pointer if
/// not found.
NodePtr FindFirstTypedNodeUnderNode(const Node &root,
                                    const std::string &type_name);

/// Templated version of FindFirstTypedNodeUnderNode() that casts the returned
/// NodePtr to the templated type (derived from Node). This always asserts on
/// failure.
template <typename T> std::shared_ptr<T> FindFirstTypedNodeUnderNode(
    const Node &root, const std::string &type_name) {
    std::shared_ptr<T> typed_node = std::dynamic_pointer_cast<T>(
        FindFirstTypedNodeUnderNode(root, type_name));
    ASSERTM(typed_node, "Node of type '" + type_name + "' not found under " +
            root.GetDesc());
    return typed_node;
}

/// Generic node-finding function. Returns a vector of all Nodes under the
/// given one (inclusive) that satisfy the given predicate.
std::vector<NodePtr> FindNodes(const NodePtr &root, const NodePredicate &func);

/// Same as FindNodes(), but returns a list of guaranteed unique
/// instances. Note that the returned nodes are sorted by name.
std::vector<NodePtr> FindUniqueNodes(const NodePtr &root,
                                     const NodePredicate &func);

/// Same as FindNodes(), but returns a list of paths to the found Nodes.
std::vector<NodePath> FindNodePaths(const SG::NodePtr &root,
                                    const NodePredicate &func);

/// Searches for a Shape with the given name in the given Node, returning a
/// pointer to it. Returns a null pointer if not found.
ShapePtr FindShapeInNode(const Node &node, const std::string &name);

/// Templated version of FindShapeInNode() that casts the returned ShapePtr to
/// the templated type (derived from Shape). This always asserts on failure.
template <typename T> std::shared_ptr<T> FindTypedShapeInNode(
    const Node &node, const std::string &name) {
    std::shared_ptr<T> shape =
        std::dynamic_pointer_cast<T>(FindShapeInNode(node, name));
    ASSERTM(shape, "Typed Shape with name '" + name + "' not found in " +
            node.GetDesc());
    return shape;
}

}  // namespace SG
