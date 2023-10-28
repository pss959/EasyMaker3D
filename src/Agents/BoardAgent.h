#pragma once

#include <functional>
#include <string>

#include "Util/Assert.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(Panel);
DECL_SHARED_PTR(BoardAgent);

/// BoardAgent is an abstract interface class that lets Panel classes operate
/// on Boards and other Panels opaquely.  It decouples dependencies between
/// BoardManager and Panel classes.
///
/// \ingroup Agents
class BoardAgent {
  public:
    using ResultFunc = std::function<void(const Str &)>;

    /// Returns the named Panel. Asserts if the name is not known.
    virtual PanelPtr GetPanel(const Str &name) const = 0;

    /// Closes the currently open Panel, passing the given string to the result
    /// function.
    virtual void ClosePanel(const Str &result) = 0;

    /// Temporarily replaces the currently open Panel with the given
    /// Panel. When the replacement Panel is closed, the replaced Panel is
    /// restored and the result_func (if not null) is invoked with the result
    /// string from the replacement Panel.
    virtual void PushPanel(const PanelPtr &panel,
                           const ResultFunc &result_func) = 0;
};
