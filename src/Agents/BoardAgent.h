#pragma once

#include <functional>
#include <string>

#include "Base/Memory.h"
#include "Util/Assert.h"

DECL_SHARED_PTR(Panel);
DECL_SHARED_PTR(BoardAgent);

/// BoardAgent is an abstract interface class that lets Panel classes operate
/// on Boards and other Panels opaquely.  It decouples dependencies between
/// BoardManager and Panel classes.
///
/// \ingroup Agents
class BoardAgent {
  public:
    typedef std::function<void(const std::string &)> ResultFunc;

    /// Returns the named Panel. Asserts if the name is not known.
    virtual PanelPtr GetPanel(const std::string &name) const = 0;

    /// Closes the currently open Panel, passing the given string to the result
    /// function.
    virtual void ClosePanel(const std::string &result) = 0;

    /// Temporarily replaces the currently open Panel with the given
    /// Panel. When the replacement Panel is closed, the replaced Panel is
    /// restored and the result_func (if not null) is invoked with the result
    /// string from the replacement Panel.
    virtual void PushPanel(const PanelPtr &panel,
                           const ResultFunc &result_func) = 0;
};
