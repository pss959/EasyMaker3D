#pragma once

#include <vector>

#include "SG/Node.h"
#include "Tools/Tool.h"

/// Finds all Tools under the given root Node and returns a vector containing
/// them.
std::vector<ToolPtr> FindTools(const SG::Node &root_node);
