#include "Models/RevSurfModel.h"

#include "Assert.h"
#include "Math/MeshBuilding.h"

void RevSurfModel::AddFields() {
    AddField(profile_);
    AddField(sweep_angle_);
    Model::AddFields();
}

void RevSurfModel::SetFieldParsed(const Parser::Field &field) {
    if (&field == &profile_) {
        // The default Profile (when read in) has the wrong start and end
        // points for a RevSurfModel. Replace it.
        Profile fixed_profile(Point2f(0, 1), Point2f(0, 0));
        fixed_profile.AddPoints(GetProfile().GetPoints());
        profile_ = fixed_profile;
    }
}

void RevSurfModel::SetProfile(const Profile &profile) {
    ASSERT(profile.IsValid(1));
    profile_ = profile;
    ProcessChange(SG::Change::kGeometry);
}

void RevSurfModel::SetSweepAngle(const Anglef &angle) {
    ASSERT(angle.Degrees() > 0 && angle.Degrees() <= 360);
    sweep_angle_ = angle;
    ProcessChange(SG::Change::kGeometry);
}

TriMesh RevSurfModel::BuildMesh() {
    // Determine the number of sides based on the complexity.
    const int num_sides = 3 + static_cast<int>(GetComplexity() * 117);
    return BuildRevSurfMesh(profile_, sweep_angle_, num_sides);
}
