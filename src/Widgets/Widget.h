#pragma once

#include <functional>
#include <string>

#include "Base/Dimensionality.h"
#include "Base/Memory.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "Util/Notifier.h"

struct DragInfo;

DECL_SHARED_PTR(Widget);

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
/// enough if SetTooltipFunc() has been called.
///
/// The GetActivation() notifier can be used to detect activation and
/// deactivation; it is passed the Widget and a flag indicating activation
/// (true) or deactivation (false).
///
/// \ingroup Widgets
class Widget : public SG::Node {
  public:
    /// Typedef for function that can be invoked to show or hide a Tooltip.
    typedef std::function<void(Widget &widget, const std::string &,
                               bool show)> TooltipFunc;

    /// Returns a Notifier that is invoked when the widget is activated or
    /// deactivated. It is passed the Widget and a flag indicating activation
    /// or deactivation.
    Util::Notifier<Widget&, bool> & GetActivation() { return activation_; }

    /// Sets a function that can be invoked by the Widget to show or hide a
    /// tooltip. The function is passed the Widget, the text string, and a flag
    /// that is true to show the tooltip and false to hide it.
    void SetTooltipFunc(const TooltipFunc &func) { tooltip_func_ = func; }

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

    /// Sets the state of hovering to on or off. Actually increments or
    /// decrements a counter so that multiple devices can hover the same
    /// Widget..
    void SetHovering(bool is_hovering);

    bool IsHovering() const { return IsHoveredState_(state_); }

    /// Allows derived classes to respond to an Event that causes hovering on
    /// it. The 3D point on the Widget is passed in. Note that this is called
    /// for each Event that results in the Widget t to become or remain
    /// hovered. The base class defines this to do nothing.
    virtual void UpdateHoverPoint(const Point3f &point) {}

    /// Sets the inactive color for the Widget.
    void SetInactiveColor(const Color &color);

    /// Returns the current inactive color for the Widget.
    const Color & GetInactiveColor() const { return inactive_color_; }

    /// Sets the active color for the Widget.
    void SetActiveColor(const Color &color);

    /// Returns the current active color for the Widget.
    const Color & GetActiveColor() const { return active_color_; }

    /// Sets a prefix string to use for all special color names accessed from
    /// the ColorMap. The base class defines this as "Widget".
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
    /// PointTarget on the Widget according to the pointer drag information in
    /// the given DragInfo. It should set position and direction to place the
    /// target (in stage coordinates) and set snapped_dims to the dimensions
    /// (if any) in which the position was snapped to a feature.
    ///
    /// The base class defines this to assert.
    virtual void PlacePointTarget(const DragInfo &info,
                                  Point3f &position, Vector3f &direction,
                                  Dimensionality &snapped_dims);

    /// If CanReceiveTarget() returns true, this can be called to place an
    /// EdgeTarget on the Widget according to the pointer drag information in
    /// the given DragInfo. It should set position0 and position1 to the
    /// new endpoints of the target (in stage coordinates). The current edge
    /// length is passed in for reference.
    ///
    /// The base class defines this to assert.
    virtual void PlaceEdgeTarget(const DragInfo &info, float current_length,
                                 Point3f &position0, Point3f &position1);

    ///@}

  protected:
    /// The constructor is protected to make this abstract.
    Widget() {}

    virtual void AddFields() override;

    /// Defines whether the Widget supports hover highlighting when the Widget
    /// is active. The base class defines this to return false.
    virtual bool SupportsActiveHovering() { return false; }

    /// Returns a flag indicating whether state changes should update the base
    /// color of the Widget. The base class defines this to return true.
    virtual bool ShouldSetBaseColor() const { return true; }

  private:
    /// Widget states. See the header comment.
    enum class State_ {
        kDisabled, kInactive, kHovered, kActive, kActiveHovered
    };

    /// \name Parsed Fields
    ///@{
    Parser::TField<Color>       inactive_color_;
    Parser::TField<Color>       active_color_;
    Parser::TField<Color>       disabled_color_;
    Parser::TField<Color>       hover_color_;
    Parser::TField<Vector3f>    hover_scale_;
    Parser::TField<std::string> tooltip_text_;
    ///@}

    /// Current state.
    State_ state_ = State_::kInactive;

    /// Hover count.
    size_t hover_count_ = 0;

    /// Function that is invoked to show or hide a Tooltip.
    TooltipFunc tooltip_func_;

    /// Saves the current scale factor before hovering.
    Vector3f saved_scale_;

    /// Prefix string used for all special color names accessed from the
    /// ColorMap.
    std::string color_name_prefix_{ "Widget" };

    /// Notifies when the widget is activated or deactivated.
    Util::Notifier<Widget&, bool> activation_;

    /// Changes the Widget's state to the given one.
    void SetState_(State_ new_state, bool invoke_callbacks);

    /// Begins or ends a hover.
    void ChangeHovering_(bool begin);

    /// If the given field has a value set, this returns it. Otherwise, it
    /// looks up the named special color in the ColorMap.
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
