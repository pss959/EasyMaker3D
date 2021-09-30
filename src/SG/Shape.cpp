#include "SG/Shape.h"

#include "Util/KLog.h"

namespace SG {

void Shape::CreateIonShape() {
    ion_shape_ = CreateSpecificIonShape();
    ion_shape_->SetLabel(GetName());
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

}  // namespace SG
