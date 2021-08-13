#pragma once

#include <string>

#include <ion/gfx/node.h>

namespace Util {

//! \name Graph Search Utilities
//!@{

//! Typedef for a path from a Node to a descendent node.
typedef std::vector<ion::gfx::NodePtr> NodePath;

//! Searches under the given root node for a node with the given name
//! (label). If found, this sets path to contain the path of nodes from the
//! root to it, inclusive, and returns true. Otherwise, it just returns false.
bool SearchForNode(const ion::gfx::NodePtr &root,
                   const std::string &name, NodePath &path);

//!@}

}  // namespace Util
