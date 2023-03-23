#pragma once

/// \file
/// This file defines types and macros for assertions to help with debugging.
///
/// \ingroup Utility

#include <string>

#include "Util/ExceptionBase.h"

// ============================================================================
// Assertions are turned off in release builds.
#if defined(RELEASE_BUILD)
#  define ASSERT(expr)
#  define ASSERTM(expr, msg)

// ============================================================================
#else

/// Exception class that is thrown when a run-time assertion fails.
///
/// \ingroup Utility
class AssertException : public ExceptionBase {
  public:
    AssertException(const std::string &expr, const std::string &file,
                    int line, const std::string &msg) :
        ExceptionBase(BuildMessage_(expr, file, line, msg)) {}
  private:
    static std::string BuildMessage_(const std::string &expr,
                                     const std::string &file,
                                     int line, const std::string &msg);
};

   /// Additional Assert macro that takes an optional message.
#  define ASSERTM(expr, msg) if (! (expr)) \
        throw AssertException(#expr, __FILE__, __LINE__, msg)

/// Conventional assert.
#  define ASSERT(expr) ASSERTM((expr), "")

// ============================================================================
#endif
