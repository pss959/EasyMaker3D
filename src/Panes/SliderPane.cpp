#include "Panes/SliderPane.h"

#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/Search.h"

void SliderPane::AddFields() {
    AddField(range_);
    AddField(precision_);
    Pane::AddFields();
}

bool SliderPane::IsValid(std::string &details) {
    if (! Pane::IsValid(details))
        return false;
    if (range_.WasSet()) {
        const auto &range = range_.GetValue();
        if (range[0] >= range[1]) {
            details = "Empty or negative range";
            return false;
        }
    }
    return true;
}

void SliderPane::AllFieldsParsed(bool is_template) {
    Pane::AllFieldsParsed(is_template);
    slider_ = SG::FindTypedNodeUnderNode<Slider1DWidget>(*this, "Slider");
    thumb_  = SG::FindNodeUnderNode(*slider_, "Thumb");

    auto func = [&](Widget &, const float &){ UpdateValue_(); };
    slider_->GetValueChanged().AddObserver(this, func);
}

float SliderPane::GetValue() const {
    const Vector2f &range = range_.GetValue();
    return Lerp(slider_->GetValue(), range[0], range[1]);
}

void SliderPane::SetValue(float value) {
    slider_->SetValue(value);
}

void SliderPane::SetSize(const Vector2f &size) {
    Pane::SetSize(size);

    // Keep the thumb the same relative size.
    thumb_->SetScale(Vector3f(1.f / size[0], 1.f / size[1], 1.f));
}

void SliderPane::UpdateValue_() {
    // Apply precision if set, then update the slider so it is in the correct
    // spot.  Disable the observer while changing the value so we don't get an
    // infinite loop.
    const float prec = precision_.GetValue();
    if (prec > 0) {
        // Get the current value in range.
        float value = GetValue();

        // Apply precision.
        value = RoundToPrecision(value, prec);

        // Set the slider.
        const Vector2f &range = range_.GetValue();
        slider_->GetValueChanged().EnableObserver(this, false);
        slider_->SetValue((value - range[0]) / (range[1] - range[0]));
        slider_->GetValueChanged().EnableObserver(this, true);
    }
}
