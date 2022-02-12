#include "Models/CylinderModel.h"

#include <algorithm>

#include "Math/MeshBuilding.h"

void CylinderModel::AddFields() {
    AddField(top_radius_);
    AddField(bottom_radius_);
    PrimitiveModel::AddFields();
}

bool CylinderModel::IsValid(std::string &details) {
    if (! PrimitiveModel::IsValid(details))
        return false;
    if (top_radius_ <= 0 || bottom_radius_ <= 0) {
        details = "Non-positive radius";
        return false;
    }
    return true;
}

void CylinderModel::SetRadius(Radius which, float radius) {
    auto &field = which == Radius::kTop ? top_radius_ : bottom_radius_;
    field = std::max(radius, kMinRadius);
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh CylinderModel::BuildMesh() {
    // Determine the number of sides based on the complexity.
    const int num_sides = 3 + static_cast<int>(GetComplexity() * 117);
    return BuildCylinderMesh(top_radius_, bottom_radius_, 2, num_sides);
}
