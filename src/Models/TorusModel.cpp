#include "Models/TorusModel.h"

#include <algorithm>

#include "Math/MeshBuilding.h"

void TorusModel::AddFields() {
    AddField(inner_radius_);
    AddField(outer_radius_);
    PrimitiveModel::AddFields();
}

void TorusModel::SetOuterRadius(float radius) {
    outer_radius_ = std::max(radius, GetMinOuterRadius());
    ProcessChange(SG::Change::kGeometry);
}

void TorusModel::SetInnerRadius(float radius) {
    inner_radius_ = std::max(radius,           kMinInnerRadius);
    outer_radius_ = std::max(GetOuterRadius(), GetMinOuterRadius());
    ProcessChange(SG::Change::kGeometry);
}

TriMesh TorusModel::BuildMesh() {
    // Determine the number of vertex rings and sectors (3-100) based on the
    // complexity.
    const int num_rings   = 3 + static_cast<int>(GetComplexity() * 97);
    const int num_sectors = num_rings;
    return BuildTorusMesh(inner_radius_, outer_radius_, num_rings, num_sectors);
}
