#include "Widgets/PlaneWidget.h"

#include "SG/Search.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/SphereWidget.h"

void PlaneWidget::CreationDone() {
    CompositeWidget::CreationDone();

    if (! IsTemplate()) {
        std::cerr << "XXXX Setting up " << GetDesc() << "\n";

        // Access the rotation and translation widgets.
        rotator_  = SG::FindTypedNodeUnderNode<SphereWidget>(*this, "Rotator");
        translator_ = SG::FindTypedNodeUnderNode<Slider1DWidget>(*this,
                                                                 "Translator");
        // Set up callbacks.
        rotator_->GetRotationChanged().AddObserver(
            this, [&](Widget &, const Rotationf &){ changed_.Notify(true); });
        translator_->GetValueChanged().AddObserver(
            this, [&](Widget &, const float &){ changed_.Notify(false); });
    }
}

void PlaneWidget::SetPlane(const Plane &plane) {
    plane_ = plane;
    // XXXX Do something.
}

void PlaneWidget::SetSize(float radius) {
    // XXXX Do something.
}

void PlaneWidget::SetTranslationRange(const Range1f &range) {
    // XXXX Do something.
}
