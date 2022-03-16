#pragma once

#include <memory>

#include "Widgets/ClickableWidget.h"

namespace Parser { class Registry; }

/// PushButtonWidget is a concrete clickable widget that responds to clicks on
/// it, invoking the Notifier. It can be used as a momentary button or a toggle
/// button.
class PushButtonWidget : public ClickableWidget {
  public:
    /// Sets a flag indicating whether the PushButtonWidget is a toggle as
    /// opposed to a momentary button. It is false by default.
    void SetIsToggle(bool is_toggle) { is_toggle_ = is_toggle; }

    /// Returns a flag indicating whether the PushButtonWidget is a toggle as
    /// opposed to a momentary button.
    bool IsToggle() const { return is_toggle_; }

    /// If IsToggle() returns true, this returns the current state of the
    /// toggle. Otherwise, it just returns false. It is false by default.
    bool GetToggleState() const { return toggle_state_; }

    /// If IsToggle() returns true, this sets the toggle state of the widget.
    /// Otherwise, it does nothing.a
    void SetToggleState(bool state);

    /// Redefines this to also toggle the state if IsToggle() is true.
    virtual void Click(const ClickInfo &info) override;

  protected:
    PushButtonWidget() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool> is_toggle_{"is_toggle", false};
    ///@}

    /// Current toggle state, which can be true only if IsToggle() is true.
    bool toggle_state_ = false;

    friend class Parser::Registry;
};

typedef std::shared_ptr<PushButtonWidget> PushButtonWidgetPtr;
