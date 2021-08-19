#pragma once

#include "ExceptionBase.h"

namespace SG {

//! Exception thrown when any SG function fails.
class Exception : public ExceptionBase {
  public:
    Exception(const std::string &msg) : ExceptionBase(msg) {}
};

}  // namespace SG
