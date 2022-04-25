#pragma once

#include <vector>

#include "Executors/Executor.h"

/// Initializes all Executor classes. Returns a vector of instances of them.
///
/// \ingroup Executors
std::vector<ExecutorPtr> InitExecutors();
