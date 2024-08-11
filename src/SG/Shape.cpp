//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/Shape.h"

#include "SG/Hit.h"
#include "SG/IonContext.h"
#include "Util/KLog.h"

namespace SG {

ion::gfx::ShapePtr Shape::SetUpIon(const IonContextPtr &ion_context) {
    if (! ion_shape_) {
        KLOG('Z', ion_context->GetIndent() << "SetUpIon for " << GetDesc());
        ion_shape_ = CreateSpecificIonShape();
        ion_shape_->SetLabel(GetName());
        ProcessChange(Change::kGeometry, *this);
    }
    return ion_shape_;
}

const Bounds & Shape::GetBounds() const {
    if (! bounds_valid_) {
        // Ask the derived class to compute bounds.
        bounds_ = ComputeBounds();
        bounds_valid_ = true;
        KLOG('b', "Updated bounds for " << GetDesc() << " to " << bounds_);
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
