#include "Panes/SliderPane.h"

#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"

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

void SliderPane::CreationDone(bool is_template) {
    Pane::CreationDone(is_template);

    if (! is_template) {
        slider_ = SG::FindTypedNodeUnderNode<Slider1DWidget>(*this, "Slider");
        thumb_  = SG::FindNodeUnderNode(*slider_, "Thumb");

        auto func = [&](Widget &, const float &){ SliderChanged_(); };
        slider_->GetValueChanged().AddObserver(this, func);
    }
}

void SliderPane::SetSize(const Vector2f &size) {
    Pane::SetSize(size);

    // Keep the thumb the same relative size.
    ASSERT(thumb_);
    thumb_->SetScale(Vector3f(1.f / size[0], 1.f / size[1], 1.f));
}

void SliderPane::SliderChanged_() {
    // Get the current value of the slider and apply the range. The slider is
    // normalized to [0,1].
    const Vector2f &range = range_.GetValue();
    float new_value = Lerp(slider_->GetValue(), range[0], range[1]);

    // If precision is specified apply it.
    const float prec = precision_.GetValue();
    if (prec > 0) {
        const float adjusted = RoundToPrecision(new_value, prec);

        // If the value changed, Update the slider so it is in the correct
        // spot.  Disable the observer while changing the value so we don't get
        // an infinite loop.
        if (adjusted != new_value) {
            new_value = adjusted;
            slider_->GetValueChanged().EnableObserver(this, false);
            slider_->SetValue((new_value - range[0]) / (range[1] - range[0]));
            slider_->GetValueChanged().EnableObserver(this, true);
        }
    }

    // Notify if changed.
    if (new_value != cur_value_) {
        cur_value_ = new_value;
        value_changed_.Notify(cur_value_);
    }
}
