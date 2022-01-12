#pragma once

#include <string>

namespace Util {

/// \name Stack Trace Utilities
///@{

/// Prints a stack trace to stderr.
void PrintStackTrace(size_t count = 20);

/// Returns a string containing a stack trace.
std::string GetStackTrace(size_t count = 20);

///@}

}  // namespace Util
