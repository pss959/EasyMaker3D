//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Models/SpinBasedModel.h"

#include "Math/Linear.h"
#include "Util/Assert.h"

void SpinBasedModel::AddFields() {
    Spin default_spin;
    AddModelField(center_.Init("center", default_spin.center));
    AddModelField(axis_.Init("axis",     default_spin.axis));
    AddModelField(angle_.Init("angle",   default_spin.angle));
    AddModelField(offset_.Init("offset", default_spin.offset));

    ScaledConvertedModel::AddFields();
}

bool SpinBasedModel::IsValid(Str &details) {
    if (! ScaledConvertedModel::IsValid(details))
        return false;
    if (! IsValidVector(axis_)) {
        details = "Zero-length spin axis";
        return false;
    }
    return true;
}

void SpinBasedModel::CreationDone() {
    ScaledConvertedModel::CreationDone();
}

void SpinBasedModel::SetSpin(const Spin &spin) {
    ASSERT(IsValidVector(spin.axis));
    center_  = spin.center;
    axis_    = spin.axis;
    angle_   = spin.angle;
    offset_  = spin.offset;
    ProcessChange(SG::Change::kGeometry, *this);
}

Spin SpinBasedModel::GetSpin() const {
    Spin spin;
    spin.center = center_;
    spin.axis   = axis_;
    spin.angle  = angle_;
    spin.offset = offset_;
    return spin;
}
