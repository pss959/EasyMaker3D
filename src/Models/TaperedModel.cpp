#include "Models/TaperedModel.h"

#include "Math/Beveler.h"  // XXXX
#include "Math/MeshUtils.h"
#include "Math/Profile.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

void TaperedModel::AddFields() {
    AddModelField(profile_points_.Init("profile_points"));
    AddModelField(taper_scale_.Init("taper_scale", 1));
    AddModelField(max_angle_.Init(
                      "max_angle",
                      Anglef::FromDegrees(TK::kDefaultMaxBevelAngle)));

    ConvertedModel::AddFields();
}

bool TaperedModel::IsValid(std::string &details) {
    if (! ConvertedModel::IsValid(details))
        return false;
    if (taper_scale_ <= 0) {
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

void TaperedModel::CreationDone() {
    ConvertedModel::CreationDone();

    if (! IsTemplate()) {
        ASSERT(taper_.profile.GetPointCount() == 2U);
        taper_.profile   = CreateProfile(profile_points_);
        taper_.scale     = taper_scale_;
        taper_.max_angle = max_angle_;
    }
}

void TaperedModel::SetTaper(const Bevel &taper) {
    taper_          = taper;
    profile_points_ = taper_.profile.GetMovablePoints();
    taper_scale_    = taper_.scale;
    max_angle_      = taper_.max_angle;
    ProcessChange(SG::Change::kGeometry, *this);
}

Profile TaperedModel::CreateProfile(const Profile::PointVec &points) {
    return Profile(Point2f(0, 1), Point2f(1, 0), points, 2);
}

void TaperedModel::SyncTransformsFromOperand(const Model &operand) {
    // Leave the scale alone.
    SetRotation(operand.GetRotation());
    SetTranslation(operand.GetTranslation());
}

void TaperedModel::SyncTransformsToOperand(Model &operand) const {
    // Leave the scale alone.
    operand.SetRotation(GetRotation());
    operand.SetTranslation(GetTranslation());
}

void TaperedModel::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    ConvertedModel::CopyContentsFrom(from, is_deep);

    // Copy the operand scale. The Taper is set in CreationDone().
    const TaperedModel &from_bev = static_cast<const TaperedModel &>(from);
    operand_scale_ = from_bev.operand_scale_;
}

TriMesh TaperedModel::ConvertMesh(const TriMesh &mesh) {
    // Apply the taper to the scaled operand mesh.
    const auto scale = GetOperandModel()->GetScale();
    return Beveler::ApplyBevel(ScaleMesh(mesh, scale),  taper_);
}
