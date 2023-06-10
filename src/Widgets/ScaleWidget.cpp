#include "Widgets/ScaleWidget.h"

#include "Math/Linear.h"
#include "SG/Search.h"
#include "Util/Tuning.h"
#include "Widgets/Slider1DWidget.h"

void ScaleWidget::AddFields() {
    AddField(mode_.Init("mode", Mode::kAsymmetric));
    AddField(use_modified_mode_.Init("use_modified_mode", false));
    AddField(limits_.Init("limits", Vector2f(TK::kScaleWidgetMinLimit,
                                             TK::kScaleWidgetMaxLimit)));

    CompositeWidget::AddFields();
}

void ScaleWidget::CreationDone() {
    CompositeWidget::CreationDone();

    if (! IsTemplate()) {
        min_slider_ = AddTypedSubWidget<Slider1DWidget>("MinSlider");
        max_slider_ = AddTypedSubWidget<Slider1DWidget>("MaxSlider");
        stick_      = SG::FindNodeUnderNode(*this, "Stick");

        // Add callbacks.
        min_slider_->GetValueChanged().AddObserver(
            this, [&](Widget &, const float &){ SliderChanged_(false); });
        max_slider_->GetValueChanged().AddObserver(
            this, [&](Widget &, const float &){ SliderChanged_(true); });

        // Disable the value-changed callbacks until activated.
        min_slider_->GetValueChanged().EnableObserver(this, false);
        max_slider_->GetValueChanged().EnableObserver(this, false);

        UpdateSlidersAndStick_();
    }
}

void ScaleWidget::SetLimits(const Vector2f &limits) {
    ASSERT(! min_slider_->IsDragging() && ! max_slider_->IsDragging());
    ASSERT(limits[1] > limits[0]);
    limits_ = limits;

    // If the current length is outside the limits, set the max value again so
    // it is clamped.
    const float length = GetLength();
    if (length < limits[0] || length > limits[1])
        SetMaxValue(GetMaxValue());
}

void ScaleWidget::SetMinValue(float value) {
    ASSERT(! min_slider_->IsDragging() && ! max_slider_->IsDragging());
    const Vector2f &limits = GetLimits();
    min_value_ = Clamp(value, max_value_ - limits[1], max_value_ - limits[0]);
    UpdateSlidersAndStick_();
}

void ScaleWidget::SetMaxValue(float value) {
    ASSERT(! min_slider_->IsDragging() && ! max_slider_->IsDragging());
    const Vector2f &limits = GetLimits();
    max_value_ = Clamp(value, min_value_ + limits[0], min_value_ + limits[1]);
    UpdateSlidersAndStick_();
}

void ScaleWidget::SubWidgetActivated(const std::string &name,
                                     bool is_activation) {
    const auto slider = name == "MinSlider" ? min_slider_ : max_slider_;

    if (is_activation) {
        slider->GetValueChanged().EnableObserver(this, true);
        InitForDrag_(slider);
    }
    else {
        slider->GetValueChanged().EnableObserver(this, false);
    }

    CompositeWidget::SubWidgetActivated(name, is_activation);
}

void ScaleWidget::SliderChanged_(bool is_max) {
    const auto slider = is_max ? max_slider_ : min_slider_;

    // If use_modified_mode_ is true, set the mode based on the flag when the
    // drag started.
    if (IsUsingModifiedMode()) {
        SetMode(slider->GetStartDragInfo().is_modified_mode ?
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
        if (slider == min_slider_) {
            max_value_ = 2 * starting_center_value_ - min_value_;
            max_slider_->SetValue(max_value_);
        }
        else {
            min_value_ = 2 * starting_center_value_ - max_value_;
            min_slider_->SetValue(min_value_);
        }
    }

    // In Asymmetric mode, only the dragged handle needs to move, so no
    // adjustments to handles are necessary.

    UpdateStick_();

    // Let all listeners know about the change.
    scale_changed_.Notify(*this, is_max);
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
    ASSERT(! min_slider_->IsDragging() && ! max_slider_->IsDragging());

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
    if (scale[0] <= 0)
        scale[0] = 1;
    trans[0] = .5f * (min_value_ + max_value_);
    stick_->SetScale(scale);
    stick_->SetTranslation(trans);
}
