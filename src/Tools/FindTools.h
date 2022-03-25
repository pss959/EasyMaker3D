#pragma once

#include <vector>

namespace SG { class Scene; }

#include "Tools/Tool.h"

/// Finds all Tools in the given Scene and returns a vector containing them.
std::vector<ToolPtr> FindTools(const SG::Scene &scene);
