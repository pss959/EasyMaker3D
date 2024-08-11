//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Executors/Executor.h"

/// Initializes all Executor classes. Returns a vector of instances of them.
///
/// \ingroup Executors
std::vector<ExecutorPtr> InitExecutors();
