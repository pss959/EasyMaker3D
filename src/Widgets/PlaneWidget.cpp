#include "Widgets/PlaneWidget.h"

#include "SG/Search.h"
#include "Widgets/AxisWidget.h"
#include "Widgets/Slider1DWidget.h"

void PlaneWidget::CreationDone() {
    CompositeWidget::CreationDone();

    if (! IsTemplate()) {
        rotator_       = AddTypedSubWidget<AxisWidget>("Axis");
        translator_    = AddTypedSubWidget<Slider1DWidget>("PlaneTranslator");
        plane_rotator_ = SG::FindNodeUnderNode(*this, "PlaneRotator");

        rotator_->SetTranslationEnabled(false);

        // Set up callbacks.
        rotator_->GetAxisChanged().AddObserver(
            this, [&](bool){ UpdatePlane_(true); });
        translator_->GetValueChanged().AddObserver(
            this, [&](Widget &, const float &){ UpdatePlane_(false); });
    }
}

void PlaneWidget::SetPlane(const Plane &plane) {
    plane_ = plane;

    rotator_->SetDirection(plane.normal);

    // Use the distance of the plane as the Slider1DWidget value without
    // notifying.
    translator_->GetValueChanged().EnableAll(false);
    translator_->SetValue(plane.distance);
    translator_->GetValueChanged().EnableAll(true);

    plane_rotator_->SetRotation(Rotationf::RotateInto(Vector3f::AxisY(),
                                                      plane.normal));
}

void PlaneWidget::SetSize(float radius) {
    const float kPlaneScale = 1.5f;
    const float plane_scale = kPlaneScale * radius;

    rotator_->SetSize(radius);
    translator_->SetScale(Vector3f(plane_scale, 1, plane_scale));
}

void PlaneWidget::SetTranslationRange(const Range1f &range) {
    translator_->SetRange(range.GetMinPoint(), range.GetMaxPoint());
}

void PlaneWidget::UpdatePlane_(bool is_rotation) {
    plane_.normal   = rotator_->GetDirection();
    plane_.distance = translator_->GetValue();

    if (is_rotation)
        plane_rotator_->SetRotation(Rotationf::RotateInto(Vector3f::AxisY(),
                                                          plane_.normal));

    plane_changed_.Notify(is_rotation);
}
