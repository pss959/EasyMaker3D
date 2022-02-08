#include "SG/Shape.h"

#include "Util/KLog.h"

namespace SG {

ion::gfx::ShapePtr Shape::SetUpIon() {
    if (! ion_shape_) {
        ion_shape_ = CreateSpecificIonShape();
        ion_shape_->SetLabel(GetName());
        ProcessChange(Change::kGeometry, *this);
    }
    return ion_shape_;
}

const Bounds & Shape::GetBounds() {
    if (! bounds_valid_) {
        // Ask the derived class to compute bounds.
        bounds_ = ComputeBounds();
        bounds_valid_ = true;
        KLOG('b', "Updated bounds for " << GetDesc()
             << " to " << bounds_.ToString());
    }
    return bounds_;
}

bool Shape::ProcessChange(Change change, const Object &obj) {
    if (! Object::ProcessChange(change, obj)) {
        return false;
    }
    else {
        bounds_valid_ = false;
        return true;
    }
}

}  // namespace SG
