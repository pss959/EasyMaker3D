#include "Models/CylinderModel.h"

#include <algorithm>

#include "Math/MeshBuilding.h"

void CylinderModel::SetTopRadius(float radius) {
    top_radius_ = std::max(radius, kMinRadius);
    ProcessChange(SG::Change::kGeometry);
}

void CylinderModel::SetBottomRadius(float radius) {
    bottom_radius_ = std::max(radius, kMinRadius);
    ProcessChange(SG::Change::kGeometry);
}

TriMesh CylinderModel::BuildMesh() {
    // Determine the number of sides based on the complexity.
    const int num_sides = 3 + static_cast<int>(GetComplexity() * 117);
    return BuildCylinderMesh(top_radius_, bottom_radius_, 2, num_sides);
}
