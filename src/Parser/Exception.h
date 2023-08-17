#pragma once

#include "Util/ExceptionBase.h"

namespace Parser {

/// Exception thrown when any Parser function fails.
///
/// \ingroup Parser
class Exception : public ExceptionBase {
  public:
    Exception(const Str &msg) : ExceptionBase(msg) {}

    Exception(const FilePath &path, const Str &msg) :
        ExceptionBase(path, "Parse error: " + msg) {}

    Exception(const FilePath &path, int line_number, const Str &msg) :
        ExceptionBase(path, line_number, "Parse error: " + msg) {}
};

}  // namespace Parser
