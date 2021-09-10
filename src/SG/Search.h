#pragma once

#include <string>

#include "Assert.h"
#include "SG/NodePath.h"
#include "SG/Typedefs.h"
#include "Util/General.h"

namespace SG {

class Scene;

//! \file
//! \name Graph Search Utilities
//! This file contains utility functions used to find SG Nodes inside scenes.
//! Each of these takes a \c ok_if_not_found flag that defaults to false. An
//! assertion will be triggered if this is set to true and the searched-for
//! Node is not found.
//!@{

//! Finds the named node in a scene, returning a path from the root node of the
//! scene to it. Returns an empty NodePath if not found.
NodePath FindNodePathInScene(const Scene &scene, const std::string &name,
                             bool ok_if_not_found = false);

//! Finds the named node in a scene, returning a pointer to it. Returns a null
//! pointer if not found.
NodePtr FindNodeInScene(const Scene &scene, const std::string &name,
                        bool ok_if_not_found = false);

//! Searches under the given root node for a node with the given name,
//! returning a path from the root to the found node. Returns an empty NodePath
//! if not found.
NodePath FindNodePathUnderNode(const NodePtr &root, const std::string &name,
                               bool ok_if_not_found = false);

//! Searches under the given root node for a node with the given name,
//! returning a pointer to it. Returns a null pointer if not found.
NodePtr FindNodeUnderNode(const NodePtr &root, const std::string &name,
                          bool ok_if_not_found = false);

//! Templated version of FindNodeInScene() that casts the returned NodePtr to
//! the templated type (derived from Node). This always asserts on failure.
template <typename T> std::shared_ptr<T> FindTypedNodeInScene(
    const Scene &scene, const std::string &name) {
    NodePtr node = FindNodeInScene(scene, name, false);
    std::shared_ptr<T> typed_node = Util::CastToDerived<T>(node);
    ASSERTM(typed_node, "Typed Node '" + name + "' not found in scene");
    return typed_node;
}

//!@}

}  // namespace SG
