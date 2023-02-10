#pragma once

#include "Base/Memory.h"
#include "Enums/Action.h"

class SessionState;
DECL_SHARED_PTR(ActionAgent);

/// ActionAgent is an abstract interface class for applying actions.
///
/// \ingroup Agents
class ActionAgent {
  public:
    /// Resets to original conditions.
    virtual void Reset() = 0;

    /// Updates from the given SessionState instance.
    virtual void UpdateFromSessionState(const SessionState &state) = 0;

    /// Returns true if the given Action can be applied.
    virtual bool CanApplyAction(Action action) const = 0;

    /// Applies the given Action. Asserts if the Action cannot be applied.
    virtual void ApplyAction(Action action) = 0;
};
