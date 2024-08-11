//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Models/TorusModel.h"

#include <algorithm>

#include "Math/MeshBuilding.h"
#include "Util/Tuning.h"

void TorusModel::AddFields() {
    AddModelField(inner_radius_.Init("inner_radius", TK::kTorusInnerRadius));
    AddModelField(outer_radius_.Init("outer_radius", 1));

    PrimitiveModel::AddFields();
}

bool TorusModel::IsValid(Str &details) {
    if (! PrimitiveModel::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    if (inner_radius_ <= 0 || outer_radius_ <= 0) {
        details = "Non-positive radius";
        return false;
    }
    if (outer_radius_ < GetMinOuterRadius()) {
        details = "Outer radius too small for inner radius";
        return false;
    }
    return true;
}

void TorusModel::SetOuterRadius(float radius) {
    outer_radius_ = std::max(radius, GetMinOuterRadius());
    ProcessChange(SG::Change::kGeometry, *this);
}

void TorusModel::SetInnerRadius(float radius) {
    inner_radius_ = std::max(radius,           TK::kMinTorusInnerRadius);
    outer_radius_ = std::max(GetOuterRadius(), GetMinOuterRadius());
    ProcessChange(SG::Change::kGeometry, *this);
}

float TorusModel::GetMinOuterRadiusForInnerRadius(float inner_radius) {
    return 2 * inner_radius + TK::kMinTorusHoleRadius;
}

TriMesh TorusModel::BuildMesh() {
    // Determine the number of vertex rings and sectors (3-100) based on the
    // complexity.
    const int num_rings   = 3 + static_cast<int>(GetComplexity() * 97);
    const int num_sectors = num_rings;
    return BuildTorusMesh(inner_radius_, outer_radius_, num_rings, num_sectors);
}
