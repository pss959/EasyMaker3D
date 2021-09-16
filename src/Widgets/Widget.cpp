#include "Widget.h"

void Widget::AddFields() {
    Node::AddFields();
    AddField(hover_color_);
    AddField(hover_scale_);
}

void Widget::AllFieldsParsed() {
    SG::Node::AllFieldsParsed();
    // Make sure there is a UniformBlock for the Lighting pass so that the
    // hover color change can be applied.
    if (! GetUniformBlockForPass("Lighting", false))
        AddUniformBlock("Lighting");
}

void Widget::SetHovering(bool is_hovering) {
    if (IsInteractionEnabled()) {
        // Change hovering only if the Widget is not active or if it is active
        // and supports active hovering.
        if (IsActiveState_(state_)) {
            if (SupportsActiveHovering()) {
                // ActivateTooltip(isHovering);
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

void Widget::SetState_(State_ new_state, bool invoke_callbacks) {
    if (new_state == state_)
        return;

    // Stop hovering if that is the current state.
    if (IsHovering())
        ChangeHovering_(false);

    // Update the material based on the new state.
    // XXXX

    // Start hovering if that is the new state.
    if (IsHoveredState_(new_state)) {
        ChangeHovering_(true);
        //ActivateTooltip(true);
    }
    else {
        //ActivateTooltip(false);
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
        SetEmissiveColor(hover_color_);
    }
    else {
        if (hover_scale_.WasSet())
            SetScale(saved_scale_);
        SetEmissiveColor(Color::Clear());
    }
}
