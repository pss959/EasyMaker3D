#pragma once

#include <functional>
#include <memory>

#include "Dimensionality.h"
#include "Items/Tooltip.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Notifier.h"

/// Widget is an abstract base class for all interactive widgets. It is derived
/// from SG::Node so that it can be placed in a scene graph.
///
/// A Widget has 4 possible states:
///  - Disabled: the Widget will not react to anything the user does.
///  - Inactive: the Widget is not being interacted with
///  - Hovered:  the user has hovered on the Widget; it can be activated
///  - Active:   the Widget is in use (e.g., a button is still pressed or a
///              slider thumb is being dragged.)
///
/// A Widget may indicate these states using color and/or scale changes.  In
/// addition, a Widget may show a Tooltip when in the Hovered state for long
/// enough.
///
/// The GetActivation() notifier can be used to detect activation and
/// deactivation; it is passed the Widget and a flag indicating activation
/// (true) or deactivation (false).
///
/// \ingroup Widgets
class Widget : public SG::Node {
  public:
    /// Typedef for function that can be invoked to enable or disable a Widget.
    typedef std::function<bool(void)> EnableFunc;

    virtual void AddFields() override;

    /// Returns a Notifier that is invoked when the widget is activated or
    /// deactivated. It is passed the Widget and a flag indicating activation
    /// or deactivation.
    Util::Notifier<Widget&, bool> & GetActivation() { return activation_; }

    /// Sets a function that can be invoked to determine whether the Widget
    /// should be enabled for interaction. This allows this decision to be
    /// separated from the code that enables or disables the Widget.
    void SetEnableFunction(const EnableFunc &func) { enable_func_ = func; }

    /// Returns a flag indicating whether the Widget should be enabled. If a
    /// function was set with SetEnableFunction(), this invokes it and returns
    /// the result. Otherwise, it always returns true.
    bool ShouldBeEnabled() const {
        return enable_func_ ? enable_func_() : true;
    }

    /// Enables or disables the Widget for interacting.
    void SetInteractionEnabled(bool enabled) {
        // If the Widget is already in the correct state, do nothing.
        if (IsInteractionEnabled() != enabled)
            SetState_(enabled ? State_::kInactive : State_::kDisabled, true);
    }

    /// Returns true if the widget is not disabled for interaction.
    bool IsInteractionEnabled() const {
        return state_ != State_::kDisabled;
    }

    /// Sets the state to indicate that the Widget is active or not. Does
    /// nothing if the Widget is disabled. Notifies of activation if notify is
    /// true (the default).
    void SetActive(bool active, bool notify = true) {
        if (IsInteractionEnabled() && active != IsActiveState_(state_))
            SetState_(active ? State_::kActive : State_::kInactive, notify);
    }

    /// Returns true if the widget is currently active.
    bool IsActive() const {
        return IsActiveState_(state_);
    }

    /// Sets the state of hovering to on or off.
    void SetHovering(bool is_hovering);

    bool IsHovering() const { return IsHoveredState_(state_); }

    /// Sets a prefix string to use for all special color names accessed
    /// from the ColorManager. The base class defines this as "Widget".
    void SetColorNamePrefix(const std::string &prefix) {
        color_name_prefix_ = prefix;
    }

    /// Sets the tooltip text to display when hovered long enough. The default
    /// text is empty, which disables tooltips.
    void SetTooltipText(const std::string &text);

    /// Redefines this to set up colors.
    virtual void PostSetUpIon() override;

    /// \name Target Interface
    ///@{

    /// Returns true if a target can be placed on this Widget. The base class
    /// defines this to return false.
    virtual bool CanReceiveTarget() const { return false; }

    /// If CanReceiveTarget() returns true, this can be called to place a
    /// PointTarget on the Widget according to the given SG::Hit, whose path
    /// ends at this Widget. It should set position and direction to place the
    /// target and set snapped_dims to the dimensions (if any) in which the
    /// position was snapped to a feature.
    ///
    /// The base class defines this to assert.
    virtual void PlacePointTarget(const SG::Hit &hit, bool is_alternate_mode,
                                  Point3f &position, Vector3f &direction,
                                  Dimensionality &snapped_dims);

    ///@}

  protected:
    /// The constructor is protected to make this abstract.
    Widget() {}

    /// Defines whether the Widget supports hover highlighting when the Widget
    /// is active. The base class defines this to return false.
    virtual bool SupportsActiveHovering() { return false; }

  private:
    /// Widget states. See the header comment.
    enum class State_ {
        kDisabled, kInactive, kHovered, kActive, kActiveHovered
    };

    /// \name Parsed Fields
    ///@{
    Parser::TField<Color>       inactive_color_{"inactive_color"};
    Parser::TField<Color>       active_color_{"active_color"};
    Parser::TField<Color>       disabled_color_{"disabled_color"};
    Parser::TField<Color>       hover_color_{"hover_color"};
    Parser::TField<Vector3f>    hover_scale_{"hover_scale", {1, 1, 1}};
    Parser::TField<std::string> tooltip_text_{"tooltip_text"};
    ///@}

    /// Current state.
    State_ state_ = State_::kInactive;

    /// Function that returns whether the Widget should be enabled.
    EnableFunc enable_func_;

    /// Saves the current scale factor before hovering.
    Vector3f saved_scale_;

    /// Prefix string used for all special color names accessed from the
    /// ColorManager.
    std::string color_name_prefix_{ "Widget" };

    /// Tooltip object. Null until the tooltip is first activated.
    TooltipPtr  tooltip_;

    /// Notifies when the widget is activated or deactivated.
    Util::Notifier<Widget&, bool> activation_;

    /// Changes the Widget's state to the given one.
    void SetState_(State_ new_state, bool invoke_callbacks);

    /// Begins or ends a hover.
    void ChangeHovering_(bool begin);

    /// If the given field has a value set, this returns it. Otherwise, it
    /// looks up the named special color in the ColorManager.
    Color GetColor_(const Parser::TField<Color> &field,
                    const std::string &name) const;

    /// Activates or deactivates the Tooltip object. Creates it first if
    /// necessary.
    void ActivateTooltip_(bool is_active);

    /// Convenience that returns true if a state represents an active Widget.
    static bool IsActiveState_(State_ state) {
        return state == State_::kActive || state == State_::kActiveHovered;
    }

    /// Convenience that returns true if a state represents a hovered Widget.
    static bool IsHoveredState_(State_ state) {
        return state == State_::kHovered || state == State_::kActiveHovered;
    }
};

typedef std::shared_ptr<Widget> WidgetPtr;
