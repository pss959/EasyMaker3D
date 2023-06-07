#include "Widgets/SpinWidget.h"

#include "SG/Search.h"
#include "SG/Torus.h"
#include "Widgets/AxisWidget.h"
#include "Widgets/DiscWidget.h"

void SpinWidget::CreationDone() {
    CompositeWidget::CreationDone();

    if (! IsTemplate()) {
        axis_ = Util::CastToDerived<AxisWidget>(AddSubWidget("Axis"));
        ring_ = Util::CastToDerived<DiscWidget>(AddSubWidget("Ring"));

        ring_transform_ = SG::FindNodeUnderNode(*this, "RingTransform");

        axis_->SetTranslationRange(Range2f(Point2f(-100, -100),
                                           Point2f(100, 100)));

        // Set up callbacks.
        axis_->GetAxisChanged().AddObserver(
            this, [&](bool is_rotation){
                UpdateSpin_(is_rotation ? ChangeType::kAxis :
                            ChangeType::kCenter); });
        ring_->GetRotationChanged().AddObserver(
            this, [&](Widget &, const Anglef &){
                UpdateSpin_(ChangeType::kAngle); });
    }
}

void SpinWidget::SetSpin(const Spin &spin) {
    //std::cerr << "XXXX SpinWidget: " << spin.ToString() << "\n";
    spin_ = spin;
    axis_->SetDirection(spin.axis);
    axis_->SetPosition(spin.center);
    ring_->SetRotationAngle(spin.angle);

    ring_transform_->SetRotation(Rotationf::RotateInto(Vector3f::AxisY(),
                                                       spin.axis));
    ring_transform_->SetTranslation(spin.center);
}

void SpinWidget::SetSize(float radius) {
    //std::cerr << "XXXX SpinWidget radius = " << radius << "\n";

    // Scale the AxisWidget.
    axis_->SetSize(radius);

    // Scale the DiscWidget parts.
    const float kRingScale  = 1.2f;
    const float kStickScale = 1.95f * kRingScale;
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
}

void SpinWidget::UpdateSpin_(ChangeType type) {
    spin_.axis   = axis_->GetDirection();
    spin_.center = axis_->GetPosition();
    spin_.angle  = ring_->GetRotationAngle();

    ring_transform_->SetRotation(Rotationf::RotateInto(Vector3f::AxisY(),
                                                       spin_.axis));
    ring_transform_->SetTranslation(spin_.center);

    spin_changed_.Notify(type);
}
