#include "Models/TorusModel.h"

#include <algorithm>

#include "Math/MeshBuilding.h"

void TorusModel::AddFields() {
    AddModelField(inner_radius_.Init("inner_radius", .2f)); // XXXX
    AddModelField(outer_radius_.Init("outer_radius", 1));

    PrimitiveModel::AddFields();
}

bool TorusModel::IsValid(std::string &details) {
    if (! PrimitiveModel::IsValid(details))
        return false;
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
    inner_radius_ = std::max(radius,           kMinInnerRadius);
    outer_radius_ = std::max(GetOuterRadius(), GetMinOuterRadius());
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh TorusModel::BuildMesh() {
    // Determine the number of vertex rings and sectors (3-100) based on the
    // complexity.
    const int num_rings   = 3 + static_cast<int>(GetComplexity() * 97);
    const int num_sectors = num_rings;
    return BuildTorusMesh(inner_radius_, outer_radius_, num_rings, num_sectors);
}
