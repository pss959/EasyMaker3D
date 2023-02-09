#pragma once

#include <string>

#include "Base/Memory.h"

DECL_SHARED_PTR(NameAgent);

/// NameAgent is an abstract interface class that manages names to guarantee
/// uniqueness.
///
/// \ingroup Agents
class NameAgent {
  public:
    /// Returns true if the given name is known.
    virtual bool Find(const std::string &name) const = 0;
};
