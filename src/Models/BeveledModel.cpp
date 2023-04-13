#include "Models/BeveledModel.h"

#include "Math/Beveler.h"
#include "Math/Profile.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

void BeveledModel::AddFields() {
    AddModelField(profile_points_.Init("profile_points"));
    AddModelField(bevel_scale_.Init("bevel_scale", 1));
    AddModelField(max_angle_.Init(
                      "max_angle",
                      Anglef::FromDegrees(TK::kDefaultMaxBevelAngle)));

    ConvertedModel::AddFields();
}

bool BeveledModel::IsValid(std::string &details) {
    if (! ConvertedModel::IsValid(details))
        return false;
    if (bevel_scale_ <= 0) {
        details = "Non-positive scale value";
        return false;
    }

    // Construct and validate the Profile if points were specified.
    if (profile_points_.WasSet()) {
        if (! CreateProfile(profile_points_).IsValid()) {
            details = "Invalid profile";
            return false;
        }
    }

    return true;
}

void BeveledModel::CreationDone() {
    ConvertedModel::CreationDone();

    if (! IsTemplate()) {
        ASSERT(bevel_.profile.GetPointCount() == 2U);
        bevel_.profile   = CreateProfile(profile_points_);
        bevel_.scale     = bevel_scale_;
        bevel_.max_angle = max_angle_;
    }
}

void BeveledModel::SetBevel(const Bevel &bevel) {
    bevel_          = bevel;
    profile_points_ = bevel_.profile.GetMovablePoints();
    bevel_scale_    = bevel_.scale;
    max_angle_      = bevel_.max_angle;
    ProcessChange(SG::Change::kGeometry, *this);
}

Profile BeveledModel::CreateProfile(const Profile::PointVec &points) {
    return Profile(Point2f(0, 1), Point2f(1, 0), points, 2);
}

TriMesh BeveledModel::ConvertMesh(const TriMesh &mesh) {
    return Beveler::ApplyBevel(mesh, bevel_);
}
