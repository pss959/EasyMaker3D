#include "Widget.h"

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
#if XXXX
    if (begin) {
        // Change color.
        Color color = ColorDict.GetColor(hoveredColorName);
        if (color != Color.clear)
            UT.ApplyEmissiveColorUnder(gameObject, color);

        // Change scale.
        if (hoveredScale != Vector3.one) {
            _savedScale = transform.localScale;
            transform.localScale =
                Vector3.Scale(hoveredScale, transform.localScale);
        }
    }
    else {
        // Restore color.
        Color color = ColorDict.GetColor(hoveredColorName);
        if (color != Color.clear)
            UT.ClearEmissiveColorUnder(gameObject);
        // Restore scale.
        if (hoveredScale != Vector3.one)
            transform.localScale = _savedScale;
    }
#endif
}
