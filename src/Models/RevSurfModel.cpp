#include "Models/RevSurfModel.h"

#include "Math/MeshBuilding.h"
#include "Util/Assert.h"

void RevSurfModel::AddFields() {
    AddModelField(profile_points_.Init("profile_points"));
    AddModelField(sweep_angle_.Init("sweep_angle", Anglef::FromDegrees(360)));

    Model::AddFields();
}

bool RevSurfModel::IsValid(std::string &details) {
    if (! Model::IsValid(details))
        return false;

    // Construct and validate the profile if points were specified.
    if (profile_points_.WasSet()) {
        Profile profile(Point2f(0, 1), Point2f(0, 0));
        profile.AddPoints(profile_points_);
        if (! profile.IsValid(1)) {
            details = "Invalid profile";
            return false;
        }
    }

    if (sweep_angle_.GetValue().Radians() == 0) {
        details = "Zero sweep angle";
        return false;
    }

    return true;
}

void RevSurfModel::CreationDone() {
    Model::CreationDone();

    if (! IsTemplate()) {
        if (profile_points_.WasSet()) {
            profile_ = Profile(Point2f(0, 1), Point2f(0, 0));
            profile_.AddPoints(profile_points_);
        }
    }
}

void RevSurfModel::SetProfile(const Profile &profile) {
    ASSERT(profile.IsValid(1));
    profile_ = profile;
    profile_points_ = profile_.GetPoints();
    ProcessChange(SG::Change::kGeometry, *this);
}

void RevSurfModel::SetSweepAngle(const Anglef &angle) {
    ASSERT(angle.Degrees() > 0 && angle.Degrees() <= 360);
    sweep_angle_ = angle;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh RevSurfModel::BuildMesh() {
    // Determine the number of sides based on the complexity.
    const int num_sides = 3 + static_cast<int>(GetComplexity() * 117);
    return BuildRevSurfMesh(profile_, sweep_angle_, num_sides);
}
