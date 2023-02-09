#pragma once

#include "Base/Memory.h"
#include "Enums/Action.h"

DECL_SHARED_PTR(ActionAgent);

/// ActionAgent is an abstract interface class for applying actions.
///
/// \ingroup Agents
class ActionAgent {
  public:
    /// Returns true if the given Action can be applied.
    virtual bool CanApplyAction(Action action) const = 0;

    /// Applies the given Action. Asserts if the Action cannot be applied.
    virtual void ApplyAction(Action action) = 0;
};
