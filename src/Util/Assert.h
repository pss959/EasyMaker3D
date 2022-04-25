#pragma once

/// \file
/// This file defines types and macros for assertions to help with debugging.
///
/// \ingroup Utility

#include <assert.h>

#include <exception>
#include <string>

#include "Util/StackTrace.h"
#include "Util/String.h"

// ============================================================================
#if defined NDEBUG
   // Assertions are turned off if NDEBUG is defined.
#  define ASSERT(expr) assert(expr)
#  define ASSERTM(expr, msg) assert(expr)

// ============================================================================
#else

/// Exception class that is thrown when a run-time assertion fails.
///
/// \ingroup Utility
class AssertException : public std::exception {
  public:
    AssertException(const std::string &expr, const std::string &file,
                    int line, const std::string &msg) {
        msg_ = file + ":" + Util::ToString(line) +
            ": Assertion failed: " + expr;
        if (! msg.empty())
            msg_ += ": " + msg;
        msg_ += "\nStack trace:\n" + Util::GetStackTrace();
    }
    const char * what() const throw() override {
        return msg_.c_str();
    }
  private:
    std::string msg_;
};

   /// Additional Assert macro that takes an optional message.
#  define ASSERTM(expr, msg) if (! (expr)) \
        throw AssertException(#expr, __FILE__, __LINE__, msg)

/// Conventional assert.
#  define ASSERT(expr) ASSERTM((expr), "")

// ============================================================================
#endif
