#include "SG/Shape.h"

namespace SG {

void Shape::SetUpIon(IonContext &context) {
    if (! ion_shape_) {
        ion_shape_ = CreateIonShape();
        ion_shape_->SetLabel(GetName());
    }
}

const Bounds & Shape::GetBounds() {
    if (! bounds_valid_) {
        // Ask the derived class to compute bounds.
        bounds_ = ComputeBounds();
        bounds_valid_ = true;
    }
    return bounds_;
}

}  // namespace SG
