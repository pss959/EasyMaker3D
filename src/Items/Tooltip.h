#pragma once

#include <memory>

#include "Defaults.h"
#include "SG/TextNode.h"

namespace Parser { class Registry; }

/// Tooltip is a derived TextNode that provides an interface to display
/// a tooltip after a delay and to always face the viewer.
class Tooltip : public SG::TextNode {
  public:
    /// Sets the delay (in seconds) to use for all tooltips.  A value of 0
    /// means no tooltips. The default is Defaults::kTooltipDelay.
    static void SetDelay(float delay) { delay_ = delay; }

    virtual bool IsValid(std::string &details) override;

    //! Shows the tooltip after the delay, unless the delay is 0.
    void ShowAfterDelay();

    //! Hides the tooltip if it is visible.
    void Hide();

  protected:
    Tooltip() {}

  private:
    /// Delay in seconds before showing a tooltip. 0 means no tooltips.
    static float delay_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<Tooltip> TooltipPtr;
