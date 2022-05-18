#include "Models/BeveledModel.h"

#include "Math/Beveler.h"
#include "Math/MeshUtils.h"
#include "Math/Profile.h"
#include "Util/Assert.h"

void BeveledModel::AddFields() {
    AddModelField(profile_points_);
    AddModelField(bevel_scale_);
    AddModelField(max_angle_);
    ConvertedModel::AddFields();
}

bool BeveledModel::IsValid(std::string &details) {
    if (! ConvertedModel::IsValid(details))
        return false;
    if (bevel_scale_ <= 0) {
        details = "Non-positive scale value";
        return false;
    }

    // Construct and validate the profile.
    Profile profile = bevel_.profile;
    profile.AddPoints(profile_points_);
    if (! profile.IsValid(0)) {
        details = "Invalid profile";
        return false;
    }

    return true;
}

void BeveledModel::CreationDone() {
    ConvertedModel::CreationDone();

    if (! IsTemplate()) {
        bevel_.profile.AddPoints(profile_points_);
        bevel_.scale     = bevel_scale_;
        bevel_.max_angle = max_angle_;
    }
}

void BeveledModel::SetBevel(const Bevel &bevel) {
    bevel_          = bevel;
    profile_points_ = bevel_.profile.GetPoints();
    bevel_scale_    = bevel_.scale;
    max_angle_      = bevel_.max_angle;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh BeveledModel::ConvertMesh(const TriMesh &original_mesh) {
    return Beveler::ApplyBevel(original_mesh, bevel_);
}
