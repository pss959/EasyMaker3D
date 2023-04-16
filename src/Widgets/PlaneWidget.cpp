#include "Widgets/PlaneWidget.h"

#include "SG/Search.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/SphereWidget.h"

void PlaneWidget::CreationDone() {
    CompositeWidget::CreationDone();

    if (! IsTemplate()) {
        // Access the rotation and translation widgets.
        rotator_  = SG::FindTypedNodeUnderNode<SphereWidget>(*this, "Rotator");
        translator_ = SG::FindTypedNodeUnderNode<Slider1DWidget>(*this,
                                                                 "Translator");
        // Access the other parts that are needed for rotating and sizing.
        arrow_rotator_ = SG::FindNodeUnderNode(*this, "ArrowRotator");
        arrow_shaft_   = SG::FindNodeUnderNode(*translator_, "Shaft");
        arrow_cone_    = SG::FindNodeUnderNode(*translator_, "Cone");

        arrow_inactive_color_ = translator_->GetInactiveColor();

        // Set up callbacks.
        rotator_->GetActivation().AddObserver(
            this, [&](Widget &, bool is_act){ Activate_(is_act); });
        translator_->GetActivation().AddObserver(
            this, [&](Widget &, bool is_act){ Activate_(is_act); });
        rotator_->GetRotationChanged().AddObserver(
            this, [&](Widget &, const Rotationf &){ RotationChanged_(); });
        translator_->GetValueChanged().AddObserver(
            this, [&](Widget &, const float &){ TranslationChanged_(); });
    }
}

void PlaneWidget::SetPlane(const Plane &plane) {
    // Use the plane normal to compute the rotation. The untransformed normal
    // direction is the +Y axis.
    rotator_->SetRotation(Rotationf::RotateInto(Vector3f::AxisY(),
                                                plane.normal));

    // Use the distance of the plane as the Slider1DWidget value without
    // notifying.
    translator_->GetValueChanged().EnableAll(false);
    translator_->SetValue(plane.distance);
    translator_->GetValueChanged().EnableAll(true);

    UpdateTranslator_();
    UpdateRotator_();
}

Plane PlaneWidget::GetPlane() const {
    return Plane(translator_->GetValue(),
                 rotator_->GetRotation() * Vector3f::AxisY());
}

void PlaneWidget::SetSize(float radius) {
    const float kArrowScale = 1.6f;
    const float kPlaneScale = 1.5f;

    rotator_->SetScale(Vector3f(kPlaneScale * radius, 1, kPlaneScale * radius));

    // Scale the arrow shaft and position the cone at the end.
    const float arrow_scale = kArrowScale * radius;
    arrow_shaft_->SetScale(Vector3f(1, arrow_scale, 1));
    arrow_cone_->SetTranslation(Vector3f(0, arrow_scale, 0));
}

void PlaneWidget::SetTranslationRange(const Range1f &range) {
    translator_->SetRange(range.GetMinPoint(), range.GetMaxPoint());
}

void PlaneWidget::HighlightArrowColor(const Color &color) {
    translator_->SetInactiveColor(color);
    translator_->SetActiveColor(color);
}

void PlaneWidget::UnhighlightArrowColor() {
    translator_->SetInactiveColor(arrow_inactive_color_);
    translator_->SetActiveColor(arrow_inactive_color_);
}

void PlaneWidget::Activate_(bool is_activation) {
    // Propagate the notification to observers of the PlaneWidget.
    GetActivation().Notify(*this, is_activation);
}

void PlaneWidget::RotationChanged_() {
    UpdateTranslator_();
    plane_changed_.Notify(true);
}

void PlaneWidget::TranslationChanged_() {
    UpdateRotator_();
    plane_changed_.Notify(false);
}

void PlaneWidget::UpdateRotator_() {
    rotator_->SetTranslation(translator_->GetValue() *
                             (rotator_->GetRotation() * Vector3f::AxisY()));
}

void PlaneWidget::UpdateTranslator_() {
    arrow_rotator_->SetRotation(rotator_->GetRotation());
}
