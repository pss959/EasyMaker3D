#include "Panes/SliderPane.h"

#include "Base/Tuning.h"
#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Widgets/Slider1DWidget.h"

void SliderPane::AddFields() {
    AddField(orientation_.Init("orientation", Orientation::kHorizontal));
    AddField(range_.Init("range",             Vector2f(0, 1)));
    AddField(precision_.Init("precision",     0));

    LeafPane::AddFields();
}

bool SliderPane::IsValid(std::string &details) {
    if (! LeafPane::IsValid(details))
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

void SliderPane::CreationDone() {
    LeafPane::CreationDone();

    if (! IsTemplate()) {
        slider_ = SG::FindTypedNodeUnderNode<Slider1DWidget>(*this, "Slider");
        thumb_  = SG::FindNodeUnderNode(*slider_, "Thumb");

        slider_->GetActivation().AddObserver(
            this, [&](Widget &, bool is_act){ SliderActivated_(is_act); });
        slider_->GetValueChanged().AddObserver(
            this, [&](Widget &, const float &){ SliderChanged_(); });
        slider_->GetValueChanged().EnableObserver(this, false);

        // Process the orientation.
        ASSERT(GetChildCount() == 1U);
        if (GetOrientation() == Orientation::kHorizontal) {
            SetWidthResizable(true);
        }
        else {
            SetHeightResizable(true);
            GetChild(0)->SetRotation(
                Rotationf::FromAxisAndAngle(Vector3f::AxisZ(),
                                            Anglef::FromDegrees(90)));
        }

        // Set up for grip drags.
        slider_->SetGripDragScale(TK::kPaneGripDragScale);
    }
}

void SliderPane::SetNormalizedSliderRange(const Vector2f &range) {
    slider_->SetRange(range[0], range[1]);
}

void SliderPane::SetValue(float new_value) {
    UpdateSliderValue_(new_value);
}

void SliderPane::SetLayoutSize(const Vector2f &size) {
    LeafPane::SetLayoutSize(size);

    // Keep the thumb the same relative size.
    ASSERT(thumb_);
    if (GetOrientation() == Orientation::kHorizontal)
        thumb_->SetScale(Vector3f(1.f / size[0], 1.f / size[1], 1.f));
    else
        thumb_->SetScale(Vector3f(1.f / size[1], 1.f / size[0], 1.f));
}

ClickableWidgetPtr SliderPane::GetActivationWidget() const {
    // Clicking anywhere on the Slider1DWidget activates the Pane.
    return slider_;
}

bool SliderPane::CanFocus() const {
    return true;
}

void SliderPane::SliderActivated_(bool is_activation) {
    slider_->GetValueChanged().EnableObserver(this, is_activation);
    activation_.Notify(is_activation);
}

void SliderPane::SliderChanged_() {
    // Get the current value of the slider and apply the range. The slider is
    // normalized to [0,1].
    const Vector2f &range = range_.GetValue();
    float new_value = Lerp(slider_->GetValue(), range[0], range[1]);
    UpdateSliderValue_(new_value);
}

float SliderPane::AdjustValue_(float value) const {
    float adjusted = value;

    // If precision is specified apply it.
    const float prec = precision_.GetValue();
    if (prec > 0)
        adjusted = RoundToPrecision(adjusted, prec);

    // Clamp to the range.
    const Vector2f &range = range_.GetValue();
    return Clamp(adjusted, range[0], range[1]);
}

void SliderPane::UpdateSliderValue_(float value) {
    // Apply precision and clamp.
    const float adjusted = AdjustValue_(value);

    if (adjusted != cur_value_) {
        cur_value_ = adjusted;

        // Update the slider so it is in the correct spot without notifying the
        // SliderPane.
        const Vector2f &range = range_.GetValue();
        slider_->GetValueChanged().EnableObserver(this, false);
        slider_->SetValue((adjusted - range[0]) / (range[1] - range[0]));
        slider_->GetValueChanged().EnableObserver(this, true);

        // Notify observers.
        value_changed_.Notify(adjusted);
    }
}
