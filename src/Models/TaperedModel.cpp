#include "Models/TaperedModel.h"

#include "Math/MeshUtils.h"
#include "Util/Assert.h"

void TaperedModel::AddFields() {
    AddModelField(axis_.Init("axis"));
    AddModelField(profile_points_.Init("profile_points"));

    ConvertedModel::AddFields();
}

bool TaperedModel::IsValid(std::string &details) {
    if (! ConvertedModel::IsValid(details))
        return false;

    // Construct and validate the Profile if points were specified.
    if (profile_points_.WasSet()) {
        if (! CreateProfile(profile_points_).IsValid()) {
            details = "Invalid profile";
            return false;
        }
    }

    return true;
}

void TaperedModel::CreationDone() {
    ConvertedModel::CreationDone();

    if (! IsTemplate()) {
        ASSERT(taper_.profile.GetPointCount() == 2U);
        taper_.axis    = axis_;
        // If there are at least 2 points specified, create a Profile from
        // them. Otherwise, leave the default taper Profile.
        if (profile_points_.GetValue().size() >= 2U)
            taper_.profile = CreateProfile(profile_points_);
    }
}

void TaperedModel::SetTaper(const Taper &taper) {
    ASSERT(taper.profile.GetType() == Profile::Type::kOpen);
    taper_          = taper;
    axis_           = taper_.axis;
    profile_points_ = taper_.profile.GetPoints();
    ProcessChange(SG::Change::kGeometry, *this);
}


Profile TaperedModel::CreateProfile(const Profile::PointVec &points) {
    return Profile(Profile::Type::kOpen, 2, points);
}

TriMesh TaperedModel::ConvertMesh(const TriMesh &mesh) {
    // Apply the taper to the scaled operand mesh.
    // XXXX return TaperMesh(mesh, taper_);
    return mesh; // XXXX
}
