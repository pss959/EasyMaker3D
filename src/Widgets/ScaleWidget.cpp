#include "Widgets/ScaleWidget.h"

#include "Base/Tuning.h"
#include "Math/Linear.h"
#include "SG/Search.h"
#include "Widgets/Slider1DWidget.h"

void ScaleWidget::AddFields() {
    AddField(mode_.Init("mode",                 Mode::kAsymmetric));
    AddField(use_alt_mode_.Init("use_alt_mode", false));
    AddField(limits_.Init("limits", Vector2f(TK::kScaleWidgetMinLimit,
                                             TK::kScaleWidgetMaxLimit)));

    Widget::AddFields();
}

void ScaleWidget::CreationDone() {
    Widget::CreationDone();

    if (! IsTemplate()) {
        min_slider_ = InitSlider_("MinSlider");
        max_slider_ = InitSlider_("MaxSlider");
        stick_ = SG::FindNodeUnderNode(*this, "Stick");
        UpdateSlidersAndStick_();
    }
}

void ScaleWidget::SetLimits(const Vector2f &limits) {
    ASSERT(! is_dragging_);
    ASSERT(limits[1] > limits[0]);
    limits_ = limits;

    // If the current length is outside the limits, set the max value again so
    // it is clamped.
    const float length = GetLength();
    if (length < limits[0] || length > limits[1])
        SetMaxValue(GetMaxValue());
}

void ScaleWidget::SetMinValue(float value) {
    ASSERT(! is_dragging_);
    const Vector2f &limits = GetLimits();
    min_value_ = Clamp(value, max_value_ - limits[1], max_value_ - limits[0]);
    UpdateSlidersAndStick_();
}

void ScaleWidget::SetMaxValue(float value) {
    ASSERT(! is_dragging_);
    const Vector2f &limits = GetLimits();
    max_value_ = Clamp(value, min_value_ + limits[0], min_value_ + limits[1]);
    UpdateSlidersAndStick_();
}

Slider1DWidgetPtr ScaleWidget::InitSlider_(const std::string &name) {
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

void ScaleWidget::SliderActivated_(const Slider1DWidgetPtr &slider,
                                   bool is_activation) {
    // Order of these is different for activation vs. deactivation: change this
    // widget's active state so that observers will be notified, and enable or
    // disable the value-changed callback on the active slider.
    if (is_activation) {
        SetActive(true, true);
        slider->GetValueChanged().EnableObserver(this, true);
        InitForDrag_(slider);
        is_dragging_ = true;
    }
    else {
        is_dragging_ = false;
        slider->GetValueChanged().EnableObserver(this, false);
        SetActive(false, true);
    }
}

void ScaleWidget::SliderChanged_(const Slider1DWidgetPtr &slider) {
    // If use_alt_mode_ is true, set the mode based on the flag when the drag
    // started.
    if (IsUsingAltMode()) {
        SetMode(slider->GetStartDragInfo().is_alternate_mode ?
                Mode::kSymmetric : Mode::kAsymmetric);
    }

    // Update the range limits based on the slider values.
    min_value_ = min_slider_->GetValue();
    max_value_ = max_slider_->GetValue();

    // In Symmetric mode, change the other handle to match.
    if (mode_ == Mode::kSymmetric) {
        // Modify the other end of the range to match the dragged slider. Note
        // that the undragged slider's observer is not active, so no need to
        // disable notifications.
        if (slider == min_slider_)
            max_slider_->SetValue(2 * starting_center_value_ - min_value_);
        else
            min_slider_->SetValue(2 * starting_center_value_ - max_value_);
    }

    // In Asymmetric mode, only the dragged handle needs to move, so no
    // adjustments to handles are necessary.

    UpdateStick_();

    // Let all listeners know about the change.
    scale_changed_.Notify(*this, slider == max_slider_);
}

void ScaleWidget::InitForDrag_(const Slider1DWidgetPtr &slider) {
    const Vector2f &limits = GetLimits();

    // Set the motion range for the moving slider.
    if (mode_ == Mode::kAsymmetric) {
        // Asymmetric mode: one handle is moving and the other is fixed.
        if (slider == max_slider_)
            max_slider_->SetRange(min_value_ + limits[0],
                                  min_value_ + limits[1]);
        else
            min_slider_->SetRange(max_value_ - limits[1],
                                  max_value_ + limits[0]);
    }
    else {
        // Symmetric mode: the handle is moving relative to the center.
        starting_center_value_ = .5f * (min_value_ + max_value_);
        min_slider_->SetRange(starting_center_value_ - .5f * limits[1],
                              starting_center_value_ - .5f * limits[0]);
        max_slider_->SetRange(starting_center_value_ + .5f * limits[0],
                              starting_center_value_ + .5f * limits[1]);
    }

    UpdateStick_();
}

void ScaleWidget::UpdateSlidersAndStick_() {
    ASSERT(! is_dragging_);

    // Set the ranges of the active sliders so the current values position the
    // handles correctly. The ranges will be updated properly when a drag
    // starts.
    const float half_length = .5f * (max_value_ - min_value_);
    min_slider_->SetRange(min_value_ - half_length, min_value_ + half_length);
    max_slider_->SetRange(max_value_ - half_length, max_value_ + half_length);

    // Update the current values to position the handles. No need to disable
    // notification to do this, since not in the middle of a drag.
    min_slider_->SetValue(min_value_);
    max_slider_->SetValue(max_value_);

    UpdateStick_();
}

void ScaleWidget::UpdateStick_() {
    Vector3f scale = stick_->GetScale();
    Vector3f trans = stick_->GetTranslation();
    scale[0] = max_value_ - min_value_;
    trans[0] = .5f * (min_value_ + max_value_);
    stick_->SetScale(scale);
    stick_->SetTranslation(trans);
}
