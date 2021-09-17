#pragma once

#include <memory>

#include "Widgets/ClickableWidget.h"

//! ToggleButtonWidget is a clickable widget that toggles between two states.
class ToggleButtonWidget : public ClickableWidget {
  public:
    //! Returns the current state of the widget. It is false by default.
    bool GetToggleState() const { return toggle_state_; }

    //! Sets the toggle state of the widget.
    void SetToggleState(bool state) { toggle_state_ = state; }

    //! Redefines this to also toggle the state.
    virtual void Click(const ClickInfo &info) override {
        toggle_state_ = ! toggle_state_;
        ClickableWidget::Click(info);
    }

  private:
    //! Current toggle state.
    bool toggle_state_ = false;
};

typedef std::shared_ptr<ToggleButtonWidget> ToggleButtonWidgetPtr;
