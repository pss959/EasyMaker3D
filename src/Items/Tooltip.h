#pragma once

#include <functional>
#include <memory>

#include "Defaults.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

class Tooltip;
typedef std::shared_ptr<Tooltip> TooltipPtr;

/// The Tooltip class is used to display a tooltip after a delay. It should
/// always face the viewer.
class Tooltip : public SG::Node {
  public:
    /// Sets the delay (in seconds) to use for all tooltips.  A value of 0
    /// means no tooltips. The default is Defaults::kTooltipDelay.
    static void SetDelay(float delay) { delay_ = delay; }

    /// Sets a function to use to create a new Tooltip instance.
    static void SetCreationFunc(const std::function<TooltipPtr()> &func) {
        creation_func_ = func;
    }

    /// Uses the creation function (which must have been set) to create a new
    /// Tooltip instance.
    static TooltipPtr Create();

    /// Sets the text to display.
    void SetText(const std::string &text);

    /// Shows the tooltip after the delay, unless the delay is 0.
    void ShowAfterDelay();

    //! Hides the tooltip if it is visible.
    void Hide();

  protected:
    Tooltip() {}

  private:
    /// Delay in seconds before showing a tooltip. 0 means no tooltips.
    static float delay_;

    /// Function invoked to create a new Tooltip instance.
    static std::function<TooltipPtr()> creation_func_;

    /// Shows or hides the tooltip.
    void SetVisible_(bool is_visible);
    friend class Parser::Registry;
};
