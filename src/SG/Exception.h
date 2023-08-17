#pragma once

#include "Util/ExceptionBase.h"

namespace SG {

/// Exception thrown when any SG function fails.
///
/// \ingroup SG
class Exception : public ExceptionBase {
  public:
    Exception(const Str &msg) : ExceptionBase(msg) {}
};

}  // namespace SG
