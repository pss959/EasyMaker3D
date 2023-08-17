#pragma once

#include <string>
#include <vector>

/// \file
/// This file contains utility functions for printing and accessing stack
/// traces when something goes wrong.
///
/// \ingroup Utility

namespace Util {

/// Returns a vector of strings representing the stack trace. Lines do not end
/// with newlines.
StrVec GetStackTrace(size_t count = 20);

/// Prints a stack trace to stderr.
void PrintStackTrace(size_t count = 20);

}  // namespace Util
