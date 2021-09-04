#pragma once

#include <memory>

#include "SG/Node.h"
#include "Util/Notifier.h"

//! Widget is an abstract base class for all interactive widgets. It is derived
//! from SG::Node so that it can be placed in a scene graph.
//!
//! A Widget has 4 possible states:
//!  - Disabled: the Widget will not react to anything the user does.
//!  - Inactive: the Widget is not being interacted with
//!  - Hovered:  the user has hovered on the Widget; it can be activated
//!  - Active:   the Widget is in use (e.g., a button is still pressed or a
//!              slider thumb is being dragged.)
//!
//! A Widget may indicate these states using color and/or scale changes.  In
//! addition, a Widget may show a Tooltip when in the Hovered state for long
//! enough.
//!
//! The Activation delegate can be used if to detech activation and deactivation
//! events; it is passed the Widget and a flag indicating activation (true) or
//! deactivation (false).
//!
//! \ingroup Widgets
class Widget : public SG::Node {
  public:
    //! Returns a Notifier that is invoked when the widget is activated or
    //! deactivated. It is passed the Widget and a flag indicating activation
    //! or deactivation.
    Util::Notifier<Widget&, bool> & GetActivation() { return activation_; }

    //! Enables or disables the Widget for interacting.
    void SetInteractionEnabled(bool enabled) {
        // If the Widget is already in the correct state, do nothing.
        if (IsInteractionEnabled() != enabled)
            SetState_(enabled ? State_::kInactive : State_::kDisabled, true);
    }

    //! Returns true if the widget is not disabled for interaction.
    bool IsInteractionEnabled() const {
        return state_ != State_::kDisabled;
    }

    //! Sets the state of hovering to on or off.
    void SetHovering(bool is_hovering);

    bool IsHovering() const {
        return IsHoveredState_(state_);
    }

  protected:
    //! The constructor is protected to make this abstract.
    Widget() {}

    //! Defines whether the Widget supports hover highlighting when the Widget
    // is active. The base class defines this to return false.
    virtual bool SupportsActiveHovering() { return false; }

  private:
    //! Widget states. See the header comment.
    enum class State_ {
        kDisabled, kInactive, kHovered, kActive, kActiveHovered
    };

    //! Current state.
    State_ state_ = State_::kInactive;

    //! Notifies when the widget is activated or deactivated.
    Util::Notifier<Widget&, bool> activation_;

    //! Changes the Widget's state to the given one.
    void SetState_(State_ new_state, bool invoke_callbacks);

    //! Begins or ends a hover.
    void ChangeHovering_(bool begin);

    //! Convenience that returns true if a state represents an active Widget.
    static bool IsActiveState_(State_ state) {
        return state == State_::kActive || state == State_::kActiveHovered;
    }

    //! Convenience that returns true if a state represents a hovered Widget.
    static bool IsHoveredState_(State_ state) {
        return state == State_::kHovered || state == State_::kActiveHovered;
    }
};

typedef std::shared_ptr<Widget> WidgetPtr;
