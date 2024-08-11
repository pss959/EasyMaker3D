//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

namespace SG { class Scene; }

#include "Tools/Tool.h"

/// Finds all Tools in the given Scene and returns a vector containing them.
///
/// \ingroup Tools
std::vector<ToolPtr> FindTools(const SG::Scene &scene);
