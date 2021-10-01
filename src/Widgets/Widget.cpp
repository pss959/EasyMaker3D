#include "Widget.h"

#include "Parser/Registry.h"

void Widget::AddFields() {
    Node::AddFields();
    AddField(hover_color_);
    AddField(hover_scale_);
    AddField(tooltip_text_);
}

bool Widget::IsValid(std::string &details) {
    if (! SG::Node::IsValid(details))
        return false;
    // Make sure there is a UniformBlock for the Lighting pass so that the
    // hover color change can be applied.
    if (! GetUniformBlockForPass("Lighting", false))
        AddUniformBlock("Lighting");
    return true;
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
        SetEmissiveColor(hover_color_);
    }
    else {
        if (hover_scale_.WasSet())
            SetScale(saved_scale_);
        SetEmissiveColor(Color::Clear());
    }
}

void Widget::ActivateTooltip_(bool is_active) {
    const std::string text = tooltip_text_;

    // Nothing to do if there is no tooltip string.
    if (text.empty())
        return;

    // Create the Tooltip and add it as a child if not already done.
    if (! tooltip_) {
        tooltip_ = Parser::Registry::CreateObject<Tooltip>("Tooltip");
        AddChild(tooltip_);
    }

    if (is_active) {
        tooltip_->SetText(text);
        tooltip_->SetTranslation(Vector3f(0, 2, 20));  // XXXX Fix this!
        tooltip_->ShowAfterDelay();
    }
    else {
        tooltip_->Hide();
    }
}
