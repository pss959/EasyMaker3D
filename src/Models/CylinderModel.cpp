#include "Models/CylinderModel.h"

#include <algorithm>

#include "Math/Linear.h"
#include "Math/MeshBuilding.h"
#include "Util/Tuning.h"

void CylinderModel::AddFields() {
    AddModelField(top_radius_.Init("top_radius",       1));
    AddModelField(bottom_radius_.Init("bottom_radius", 1));

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

void CylinderModel::SetTopRadius(float radius) {
    top_radius_ = radius;
    ProcessChange(SG::Change::kGeometry, *this);
}

void CylinderModel::SetBottomRadius(float radius) {
    bottom_radius_ = radius;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh CylinderModel::BuildMesh() {
    // Determine the number of sides based on the complexity.
    const int num_sides = LerpInt(GetComplexity(), 3, TK::kMaxCylinderSides);
    std::cerr << "XXXX TR=" << top_radius_ << " BR=" << bottom_radius_
              << " NS=" << num_sides << "\n";
    return BuildCylinderMesh(top_radius_, bottom_radius_, 2, num_sides);
}

void CylinderModel::CreationDone() {
    PrimitiveModel::CreationDone();
}
