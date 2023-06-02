#include "Widgets/PlaneWidget.h"

#include "SG/Search.h"
#include "Widgets/AxisWidget.h"
#include "Widgets/Slider1DWidget.h"

void PlaneWidget::CreationDone() {
    CompositeWidget::CreationDone();

    if (! IsTemplate()) {
        // Access the rotation and translation widgets.
        rotator_  = Util::CastToDerived<AxisWidget>(AddSubWidget("AxisWidget"));
        translator_ = Util::CastToDerived<Slider1DWidget>(
            AddSubWidget("PlaneTranslator"));

        rotator_->SetTranslationEnabled(false);

        // Set up callbacks.
        rotator_->GetAxisChanged().AddObserver(
            this, [&](bool){ UpdatePlane_(true); });
        translator_->GetValueChanged().AddObserver(
            this, [&](Widget &, const float &){ UpdatePlane_(true); });
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

    translator_->SetRotation(Rotationf::RotateInto(Vector3f::AxisY(),
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
        translator_->SetRotation(Rotationf::RotateInto(Vector3f::AxisY(),
                                                       plane_.normal));

    plane_changed_.Notify(is_rotation);
}
