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
        // Access the other parts that are needed for sizing.
        arrow_shaft_ = SG::FindNodeUnderNode(*translator_, "Shaft");
        arrow_cone_  = SG::FindNodeUnderNode(*translator_, "Cone");
    }
}

void PlaneWidget::SetPlane(const Plane &plane) {
    plane_ = plane;
    // XXXX Do something.
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
    // XXXX Do something.
}
