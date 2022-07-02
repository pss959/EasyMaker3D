#pragma once

#include <functional>
#include <string>

#include "Base/Memory.h"
#include "Util/Assert.h"
#include "Util/General.h"

DECL_SHARED_PTR(Panel);
DECL_SHARED_PTR(PanelHelper);

/// PanelHelper is an abstract class that lets Panel classes operate on Boards
/// and other Panels opaquely.  It decouples dependencies between BoardManager
/// and Panel classes.
///
/// \ingroup Panels
class PanelHelper {
  public:
    typedef std::function<void(const PanelPtr &)>    InitFunc;
    typedef std::function<void(const std::string &)> ResultFunc;

    /// Returns the named Panel. Asserts if the name is not known.
    virtual PanelPtr GetPanel(const std::string &name) const = 0;

    /// Same as GetPanel(), but requires that the Panel is of the given derived
    /// type. Asserts if not found.
    template <typename T>
    std::shared_ptr<T> GetTypedPanel(const std::string &name) const {
        auto panel = Util::CastToDerived<T>(GetPanel(name));
        ASSERT(panel);
        return panel;
    }

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
