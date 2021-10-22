#include "Widgets/Widget.h"

#include "Assert.h"
#include "Managers/ColorManager.h"

void Widget::AddFields() {
    SG::Node::AddFields();
    AddField(inactive_color_);
    AddField(active_color_);
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
                ActivateTooltip_(is_hovering);
                State_ new_state = is_hovering ? State_::kActiveHovered :
                    State_::kActive;
                SetState_(new_state, false);
            }
        }
        else {
            SetState_(is_hovering ? State_::kHovered : State_::kInactive,
                      false);
        }
    }
}

void Widget::SetTooltipText(const std::string &text) {
    tooltip_text_ = text;
    if (tooltip_)
        tooltip_->SetText(text);
}

ion::gfx::NodePtr Widget::SetUpIon(
    const SG::IonContextPtr &ion_context,
    const std::vector<ion::gfx::ShaderProgramPtr> &programs) {
    // Let the base class set up first.
    ion::gfx::NodePtr ion_node = SG::Node::SetUpIon(ion_context, programs);

    // Set the base color to the inactive color.
    SetBaseColor(GetColor_(inactive_color_, "WidgetInactiveColor"));

    return ion_node;
}

void Widget::PlacePointTarget(const SG::Hit &hit, bool is_alternate_mode,
                              Point3f &position, Vector3f &direction,
                              Dimensionality &snapped_dims) {
    ASSERTM(false, "Widget::PlacePointTarget() should not be called");
}

void Widget::SetState_(State_ new_state, bool invoke_callbacks) {
    if (new_state == state_)
        return;

    // Stop hovering if that is the current state.
    if (IsHovering())
        ChangeHovering_(false);

    // Update the base color based on the new state.
    SetBaseColor(new_state == State_::kActive ?
                 GetColor_(active_color_,   "WidgetActiveColor") :
                 GetColor_(inactive_color_, "WidgetInactiveColor"));

    // Start hovering if that is the new state.
    if (IsHoveredState_(new_state)) {
        ChangeHovering_(true);
        ActivateTooltip_(true);
    }
    else {
        ActivateTooltip_(false);
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
        SetEmissiveColor(GetColor_(hover_color_, "WidgetHoverColor"));
    }
    else {
        if (hover_scale_.WasSet())
            SetScale(saved_scale_);
        SetEmissiveColor(Color::Clear());
    }
}

Color Widget::GetColor_(const Parser::TField<Color> &field,
                        const std::string &name) const {
    return field.WasSet() ? field : ColorManager::GetSpecialColor(name);
}

void Widget::ActivateTooltip_(bool is_active) {
    const std::string text = tooltip_text_;

    // Nothing to do if there is no tooltip string.
    if (text.empty())
        return;

    // Create the Tooltip and add it as a child if not already done.
    if (! tooltip_) {
        tooltip_ = Tooltip::Create();
        AddChild(tooltip_);
    }

    if (is_active) {
        tooltip_->SetText(text);
        tooltip_->ShowAfterDelay();
    }
    else {
        tooltip_->Hide();
    }
}
