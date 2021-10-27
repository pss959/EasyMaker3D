#include "SG/Shape.h"

#include "Util/KLog.h"

namespace SG {

ion::gfx::ShapePtr Shape::SetUpIon() {
    if (! ion_shape_) {
        ion_shape_ = CreateSpecificIonShape();
        ion_shape_->SetLabel(GetName());
    }
    return ion_shape_;
}

const Bounds & Shape::GetBounds() {
    if (! bounds_valid_) {
        // Ask the derived class to compute bounds.
        bounds_ = ComputeBounds();
        bounds_valid_ = true;
        KLOG('b', "Updated bounds for " << GetDesc() << " to " << bounds_);
    }
    return bounds_;
}

void Shape::ProcessChange(Change change) {
    bounds_valid_ = false;
    Object::ProcessChange(change);
}

}  // namespace SG
