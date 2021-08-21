#pragma once

#include <string>

#include "SG/NodePath.h"
#include "SG/Typedefs.h"

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

//!@}

}  // namespace SG
