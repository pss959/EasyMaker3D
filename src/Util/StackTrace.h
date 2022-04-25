#pragma once

#include <string>

/// \file
/// This file contains utility functions for printing and accessing stack
/// traces when something goes wrong.
///
/// \ingroup Utility

namespace Util {

/// Prints a stack trace to stderr.
void PrintStackTrace(size_t count = 20);

/// Returns a string containing a stack trace.
std::string GetStackTrace(size_t count = 20);

}  // namespace Util
