#include "Widgets/WheelWidget.h"

#include "SG/Search.h"
#include "SG/Torus.h"
#include "Widgets/DiscWidget.h"

void WheelWidget::CreationDone() {
    CompositeWidget::CreationDone();

    if (! IsTemplate()) {
        rotator_ = Util::CastToDerived<DiscWidget>(AddSubWidget("Rotator"));

        // Set up callbacks.
        rotator_->GetRotationChanged().AddObserver(
            this, [&](Widget &, const Anglef &){ rotation_changed_.Notify(); });
    }
}

void WheelWidget::SetSize(float radius) {
    const float kWheelScale = 1.5f;

    // Scale the Wheel parts.
    const auto x_stick  = SG::FindNodeUnderNode(*rotator_, "XStick");
    const auto z_stick  = SG::FindNodeUnderNode(*rotator_, "ZStick");
    auto x_scale = x_stick->GetScale();
    auto z_scale = z_stick->GetScale();
    x_scale[0] = z_scale[2] = .9f * kWheelScale * radius;
    x_stick->SetScale(x_scale);
    z_stick->SetScale(z_scale);

    const auto ring  = SG::FindNodeUnderNode(*rotator_, "Ring");
    const auto torus = SG::FindTypedShapeInNode<SG::Torus>(*ring, "Torus");
    torus->SetOuterRadius(.5f * kWheelScale * radius);
}

void WheelWidget::SetRotationAngle(const Anglef &angle) {
    rotator_->SetRotationAngle(angle);
}

Anglef WheelWidget::GetRotationAngle() const {
    return rotator_->GetRotationAngle();
}
