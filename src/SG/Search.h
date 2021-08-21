#pragma once

#include <string>

#include "SG/NodePath.h"
#include "SG/Typedefs.h"

namespace SG {

class Scene;

//! \file
//! \name Graph Search Utilities
//! This file contains utility functions used to find SG Nodes inside scenes.
//!@{

//! Finds the named node in a scene, returning a path from the root node of the
//! scene to it. Returns an empty NodePath if not found.
NodePath FindNodePathInScene(const Scene &scene, const std::string &name);

//! Finds the named node in a scene, returning a pointer to it. Returns a null
//! pointer if not found.
NodePtr FindNodeInScene(const Scene &scene, const std::string &name);

//! Searches under the given root node for a node with the given name,
//! returning a path from the root to the found node. Returns an empty NodePath
//! if not found.
NodePath FindNodePathUnderNode(const NodePtr &root, const std::string &name);

//! Searches under the given root node for a node with the given name,
//! returning a pointer to it. Returns a null pointer if not found.
NodePtr FindNodeUnderNode(const NodePtr &root, const std::string &name);

//!@}

}  // namespace SG
