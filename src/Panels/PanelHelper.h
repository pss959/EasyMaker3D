#pragma once

#include <functional>
#include <memory>
#include <string>

class Panel;

/// PanelHelper is an abstract class that allows the PanelManager and
/// individual derived Panel classes to operate more effectively, allowing a
/// Panel to open a different Panel and be informed about its results.
///
/// This class is used to decouple dependencies between PanelManager and Panels.
class PanelHelper {
  public:
    typedef std::function<void(Panel &)> InitFunc;
    typedef std::function<void(Panel &, const std::string &)> ResultFunc;

    /// Closes the currently open Panel, returning the given string.
    virtual void Close(const std::string &result) = 0;

    /// Temporarily replaces the currently open Panel with the named Panel. The
    /// init_func (if not null) is invoked to initialize the new Panel. When
    /// the replacement Panel is closed, the replaced Panel is restored and the
    /// result_func (if not null) is invoked with the result string from the
    /// replacement Panel.
    virtual void Replace(const std::string &panel_name,
                         const InitFunc &init_func,
                         const ResultFunc &result_func) = 0;
};

typedef std::shared_ptr<PanelHelper> PanelHelperPtr;
