#include "Models/BeveledModel.h"

#include "Math/Beveler.h"
#include "Math/MeshUtils.h"
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
    ASSERT(bevel.profile.GetType() == Profile::Type::kFixed);
    bevel_          = bevel;
    profile_points_ = bevel_.profile.GetMovablePoints();
    bevel_scale_    = bevel_.scale;
    max_angle_      = bevel_.max_angle;
    ProcessChange(SG::Change::kGeometry, *this);
}

Profile BeveledModel::CreateProfile(const Profile::PointVec &points) {
    return Profile::CreateFixedProfile(Point2f(0, 1), Point2f(1, 0), 2, points);
}

void BeveledModel::SyncTransformsFromOperand(const Model &operand) {
    // Leave the scale alone.
    SetRotation(operand.GetRotation());
    SetTranslation(operand.GetTranslation());
}

void BeveledModel::SyncTransformsToOperand(Model &operand) const {
    // Leave the scale alone.
    operand.SetRotation(GetRotation());
    operand.SetTranslation(GetTranslation());
}

void BeveledModel::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    ConvertedModel::CopyContentsFrom(from, is_deep);

    // Copy the operand scale. The Bevel is set in CreationDone().
    const BeveledModel &from_bev = static_cast<const BeveledModel &>(from);
    operand_scale_ = from_bev.operand_scale_;
}

TriMesh BeveledModel::ConvertMesh(const TriMesh &mesh) {
    // Apply the bevel to the scaled operand mesh.
    const auto scale = GetOperandModel()->GetScale();
    return Beveler::ApplyBevel(ScaleMesh(mesh, scale),  bevel_);
}
