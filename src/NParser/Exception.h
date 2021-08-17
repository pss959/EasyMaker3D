#pragma once

#include "ExceptionBase.h"

namespace NParser {

//! Exception thrown when any NParser function fails.
class Exception : public ExceptionBase {
  public:
    Exception(const std::string &msg) : ExceptionBase(msg) {}

    Exception(const Util::FilePath &path, const std::string &msg) :
        ExceptionBase(path, "Parse error: " + msg) {}

    Exception(const Util::FilePath &path, int line_number,
              const std::string &msg) :
        ExceptionBase(path, line_number, "Parse error: " + msg) {}
};

}  // namespace NParser
