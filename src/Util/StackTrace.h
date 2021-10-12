#pragma once

#include <string>

namespace Util {

/// \name Stack Trace Utilities
///@{

/// Prints a stack trace to stderr.
void PrintStackTrace();

/// Returns a string containing a stack trace.
std::string GetStackTrace();

///@}

}  // namespace Util
