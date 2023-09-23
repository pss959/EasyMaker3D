#include "Widgets/SpinWidget.h"

#include "SG/Search.h"
#include "SG/Torus.h"
#include "Widgets/AxisWidget.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/Slider1DWidget.h"

void SpinWidget::CreationDone() {
    CompositeWidget::CreationDone();

    if (! IsTemplate()) {
        axis_   = AddTypedSubWidget<AxisWidget>("Axis");
        ring_   = AddTypedSubWidget<DiscWidget>("Ring");
        offset_ = AddTypedSubWidget<Slider1DWidget>("Offset");

        ring_transform_ = SG::FindNodeUnderNode(*this, "RingTransform");

        axis_->SetTranslationRange(Range2f(Point2f(-40, -40), Point2f(40, 40)));
        offset_->SetRange(-100, 100);

        // Set up callbacks.
        axis_->GetAxisChanged().AddObserver(
            this, [&](bool is_rotation){
                UpdateSpin_(is_rotation ? ChangeType::kAxis :
                            ChangeType::kCenter); });
        ring_->GetRotationChanged().AddObserver(
            this, [&](Widget &, const Anglef &){
                UpdateSpin_(ChangeType::kAngle); });
        offset_->GetValueChanged().AddObserver(
            this, [&](Widget &, const float &){
                UpdateSpin_(ChangeType::kOffset); });
    }
}

void SpinWidget::SetSpin(const Spin &spin) {
    spin_ = spin;
    axis_->SetDirection(spin.axis);
    axis_->SetPosition(spin.center);
    ring_->SetRotationAngle(spin.angle);

    offset_->GetValueChanged().EnableAll(false);
    offset_->SetValue(spin.offset);
    offset_->GetValueChanged().EnableAll(true);

    ring_transform_->SetRotation(Rotationf::RotateInto(Vector3f::AxisY(),
                                                       spin.axis));
    ring_transform_->TranslateTo(spin.center);
}

void SpinWidget::SetSize(float radius) {
    const float kRingScale   = 1.2f;
    const float kStickScale  = 1.95f * kRingScale;
    const float kOffsetScale = 1.1f;

    // Scale the AxisWidget.
    axis_->SetSize(radius);

    // Scale the DiscWidget parts.
    const auto x_stick  = SG::FindNodeUnderNode(*ring_, "XStick");
    const auto z_stick  = SG::FindNodeUnderNode(*ring_, "ZStick");
    auto x_scale = x_stick->GetScale();
    auto z_scale = z_stick->GetScale();
    x_scale[0] = z_scale[2] = kStickScale * radius;
    x_stick->SetScale(x_scale);
    z_stick->SetScale(z_scale);
    const auto ring  = SG::FindNodeUnderNode(*ring_, "Ring");
    const auto torus = SG::FindTypedShapeInNode<SG::Torus>(*ring, "Torus");
    torus->SetOuterRadius(kRingScale * radius);

    // Scale and position the offset Slider1DWidget.
    const auto offset_pos = SG::FindNodeUnderNode(*this, "OffsetPosition");
    offset_pos->SetTranslation(Vector3f(kOffsetScale * radius, 0, 0));
}

void SpinWidget::UpdateSpin_(ChangeType type) {
    spin_.axis   = axis_->GetDirection();
    spin_.center = axis_->GetPosition();
    spin_.angle  = ring_->GetRotationAngle();
    spin_.offset = offset_->GetValue();

    ring_transform_->SetRotation(Rotationf::RotateInto(Vector3f::AxisY(),
                                                       spin_.axis));
    ring_transform_->TranslateTo(spin_.center);

    spin_changed_.Notify(type);
}
