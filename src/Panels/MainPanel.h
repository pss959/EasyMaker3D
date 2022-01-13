#pragma once

#include <functional>
#include <string>

#include "Panels/Panel.h"

/// MainPanel is derived from Panel and serves as an abstract base class
/// that provides some conveniences for derived Panel classes.
class MainPanel : public Panel {
  protected:
    MainPanel() {}

    /// Convenience that opens a DialogPanel to display the given message along
    /// with an "OK" button that invokes the given function (if not null).
    void DisplayMessage(const std::string &message,
                        const std::function<void(void)> &func);

    /// Convenience that opens a DialogPanel to ask the given question and get
    /// a "Yes" or "No" result, which is passed to the given function.
    void AskQuestion(const std::string &question,
                     const std::function<void(const std::string &)> &func);
};
