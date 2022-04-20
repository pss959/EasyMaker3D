#include "Widgets/Widget.h"

#include "Managers/ColorManager.h"
#include "Util/Assert.h"

void Widget::AddFields() {
    SG::Node::AddFields();
    AddField(inactive_color_);
    AddField(active_color_);
    AddField(disabled_color_);
    AddField(hover_color_);
    AddField(hover_scale_);
    AddField(tooltip_text_);
}

void Widget::SetHovering(bool is_hovering) {
    if (IsInteractionEnabled()) {
        // Change hovering only if the Widget is not active or if it is active
        // and supports active hovering.
        if (IsActiveState_(state_)) {
            if (SupportsActiveHovering()) {
                const State_ new_state = is_hovering ?
                    State_::kActiveHovered : State_::kActive;
                if (new_state != state_)
                    SetState_(new_state, false);
            }
        }
        else {
            SetState_(is_hovering ? State_::kHovered : State_::kInactive,
                      false);
        }
    }
}

void Widget::SetInactiveColor(const Color &color) {
    inactive_color_ = color;
    if (state_ == State_::kInactive && GetIonContext())
        SetBaseColor(color);
}

void Widget::SetActiveColor(const Color &color) {
    active_color_ = color;
    if (state_ == State_::kActive && GetIonContext())
        SetBaseColor(color);
}

void Widget::SetTooltipText(const std::string &text) {
    tooltip_text_ = text;
}

void Widget::PostSetUpIon() {
    SG::Node::PostSetUpIon();

    // Set the base color to the inactive color.
    if (ShouldSetBaseColor())
        SetBaseColor(GetColor_(inactive_color_, "InactiveColor"));
}

void Widget::PlacePointTarget(const DragInfo &info,
                              Point3f &position, Vector3f &direction,
                              Dimensionality &snapped_dims) {
    ASSERTM(false, "Widget::PlacePointTarget() should not be called");
}

void Widget::PlaceEdgeTarget(const DragInfo &info, float current_length,
                             Point3f &position0, Point3f &position1) {
    ASSERTM(false, "Widget::PlaceEdgeTarget() should not be called");
}

void Widget::SetState_(State_ new_state, bool invoke_callbacks) {
    if (new_state == state_)
        return;

    // Stop hovering if that is the current state.
    if (IsHovering()) {
        ChangeHovering_(false);
        ActivateTooltip_(false);
    }

    // Update the base color based on the new state unless the derived class
    // says not to.
    if (ShouldSetBaseColor()) {
        if (new_state == State_::kDisabled)
            SetBaseColor(GetColor_(disabled_color_, "DisabledColor"));
        else if (new_state == State_::kActive)
            SetBaseColor(GetColor_(active_color_,   "ActiveColor"));
        else
            SetBaseColor(GetColor_(inactive_color_, "InactiveColor"));
    }

    // Start hovering if that is the new state.
    if (IsHoveredState_(new_state)) {
        ChangeHovering_(true);
        ActivateTooltip_(true);
    }

    // Invoke callbacks if requested.
    if (invoke_callbacks) {
        bool was_active = IsActiveState_(state_);
        bool is_active  = IsActiveState_(new_state);
        if (was_active && ! is_active)
            activation_.Notify(*this, false);
        else if (! was_active && is_active)
            activation_.Notify(*this, true);
    }

    state_ = new_state;
}

void Widget::ChangeHovering_(bool begin) {
    if (begin) {
        if (hover_scale_.WasSet()) {
            saved_scale_ = GetScale();
            SetScale(hover_scale_ * saved_scale_);
        }
        SetEmissiveColor(GetColor_(hover_color_, "HoverColor"));
    }
    else {
        if (hover_scale_.WasSet())
            SetScale(saved_scale_);
        SetEmissiveColor(Color::Clear());
    }
}

Color Widget::GetColor_(const Parser::TField<Color> &field,
                        const std::string &name) const {
    return field.WasSet() ? field :
        ColorManager::GetSpecialColor(color_name_prefix_ + name);
}

void Widget::ActivateTooltip_(bool is_active) {
    const std::string text = tooltip_text_;

    // Nothing to do if there is no tooltip string or tooltip function.
    if (tooltip_func_ && ! text.empty())
        tooltip_func_(*this, text, is_active);
}
