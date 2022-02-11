#include "Widgets/RangeWidget.h"

#include "Math/Linear.h"
#include "SG/Search.h"

void RangeWidget::AddFields() {
    AddField(mode_);
    AddField(length_limits_);
    Widget::AddFields();
}

void RangeWidget::CreationDone() {
    Widget::CreationDone();

    if (! IsTemplate()) {
        min_slider_ = InitSlider_("MinSlider");
        max_slider_ = InitSlider_("MaxSlider");
        stick_ = SG::FindNodeUnderNode(*this, "Stick");
        UpdateSliderRanges_();
    }
}

void RangeWidget::SetLengthLimits(const Vector2f &limits) {
    // The RangeWidget must not be in the middle of a drag.
    ASSERT(! min_slider_->GetValueChanged().IsObserverEnabled(this) &&
           ! max_slider_->GetValueChanged().IsObserverEnabled(this));

    length_limits_ = limits;
    ClampLength_();
    UpdateSliderRanges_();
    UpdateStick_();
}

void RangeWidget::SetRange(const Vector2f &range) {
    // The RangeWidget must not be in the middle of a drag.
    ASSERT(! min_slider_->GetValueChanged().IsObserverEnabled(this) &&
           ! max_slider_->GetValueChanged().IsObserverEnabled(this));

    min_slider_->SetValue(range[0]);
    max_slider_->SetValue(range[1]);
    ClampLength_();
    UpdateSliderRanges_();
    UpdateStick_();
}

void RangeWidget::ClampLength_() {
    const Vector2f  range  = GetRange();
    const Vector2f &limits = GetLengthLimits();

    const float clamped = Clamp(range[1],
                                range[0] + limits[0], range[0] + limits[1]);
    if (range[1] != clamped)
        max_slider_->SetValue(clamped);
}

Slider1DWidgetPtr RangeWidget::InitSlider_(const std::string &name) {
    auto slider = SG::FindTypedNodeUnderNode<Slider1DWidget>(*this, name);

    slider->GetActivation().AddObserver(
        this, [&, slider](Widget &, bool is_act){
            SliderActivated_(slider, is_act); });
    slider->GetValueChanged().AddObserver(
        this, [&, slider](Widget &, const float &){ SliderChanged_(slider); });

    // Disable the value-changed callback until activated.
    slider->GetValueChanged().EnableObserver(this, false);

    return slider;
}

void RangeWidget::UpdateSliderRanges_() {
    const Vector2f range       = GetRange();
    const float    half_length = .5f * (range[1] - range[0]);

    // Set the ranges of the active sliders so the current values position the
    // handles correctly. The ranges will be updated properly when a drag
    // starts.
    min_slider_->SetRange(range[0] - half_length, range[0] + half_length);
    max_slider_->SetRange(range[1] - half_length, range[1] + half_length);

    UpdateStick_();
}

void RangeWidget::SliderActivated_(const Slider1DWidgetPtr &slider,
                                   bool is_activation) {
    // Change this widget's active state so that observers will be notified.
    SetActive(is_activation, true);

    // Enable or disable the value-changed callback on the active slider.
    slider->GetValueChanged().EnableObserver(this, is_activation);

    if (is_activation)
        InitForDrag_(slider);
}

void RangeWidget::SliderChanged_(const Slider1DWidgetPtr &slider) {
    // In Asymmetric mode, only the dragged handle needs to move, so no
    // adjustments to handles are necessary.

    // Symmetric mode: change the other handle to match.
    if (mode_ == Mode::kSymmetric) {
        // Modify the other end of the range to match the dragged slider. Note
        // that the undragged slider's observer is not active.
        if (slider == min_slider_)
            max_slider_->SetValue(
                2 * starting_center_value_ - min_slider_->GetValue());
        else
            min_slider_->SetValue(
                2 * starting_center_value_ - max_slider_->GetValue());
    }

    UpdateStick_();

    // Let all listeners know about the change.
    range_changed_.Notify(*this, slider == max_slider_);
}

void RangeWidget::InitForDrag_(const Slider1DWidgetPtr &slider) {
    const Vector2f range   = GetRange();
    const Vector2f &limits = GetLengthLimits();

    // Set the motion range for the moving slider.
    if (mode_ == Mode::kAsymmetric) {
        // Asymmetric mode: one handle is moving and the other is fixed.
        if (slider == max_slider_)
            max_slider_->SetRange(range[0] + limits[0], range[0] + limits[1]);
        else
            min_slider_->SetRange(range[1] - limits[1], range[1] - limits[0]);
    }
    else {
        // Symmetric mode: the handle is moving relative to the center.
        starting_center_value_ = .5f * (range[0] + range[1]);

        min_slider_->SetRange(starting_center_value_ - .5f * limits[1],
                              starting_center_value_ - .5f * limits[0]);
        max_slider_->SetRange(starting_center_value_ + .5f * limits[0],
                              starting_center_value_ + .5f * limits[1]);
    }

    UpdateStick_();
}

// Update the stick to span the handles.
void RangeWidget::UpdateStick_() {
    const Vector2f range = GetRange();

    Vector3f scale = stick_->GetScale();
    Vector3f trans = stick_->GetTranslation();
    scale[0] = range[1] - range[0];
    trans[0] = .5f * (range[0] + range[1]);
    stick_->SetScale(scale);
    stick_->SetTranslation(trans);
}
