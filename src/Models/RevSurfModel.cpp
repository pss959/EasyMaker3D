#include "Models/RevSurfModel.h"

#include "Assert.h"
#include "Math/MeshBuilding.h"

void RevSurfModel::SetProfile(const Profile &profile) {
    profile_ = profile;
    ProcessChange(SG::Change::kGeometry);
}

void RevSurfModel::SetSweepAngle(const Anglef &angle) {
    ASSERT(angle.Degrees() > 0 && angle.Degrees() <= 360);
    sweep_angle_ = angle;
    ProcessChange(SG::Change::kGeometry);
}

TriMesh RevSurfModel::BuildMesh() {
    ASSERT(profile_.IsValid(1));

    // Determine the number of sides based on the complexity.
    const int num_sides = 3 + static_cast<int>(GetComplexity() * 117);
    return BuildRevSurfMesh(profile_, sweep_angle_, num_sides);
}
