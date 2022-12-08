#include "Models/BeveledModel.h"

#include "Base/Tuning.h"
#include "Math/Beveler.h"
#include "Math/MeshUtils.h"
#include "Math/Profile.h"
#include "Util/Assert.h"

void BeveledModel::AddFields() {
    AddModelField(profile_points_.Init("profile_points"));
    AddModelField(bevel_scale_.Init("bevel_scale", 1));
    AddModelField(max_angle_.Init("max_angle",
                                  Anglef::FromDegrees(TK::kMaxBevelAngle)));

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

TriMesh BeveledModel::BuildMesh() {
    // Scale the original mesh and then apply the bevel.
    ASSERT(GetOriginalModel());
    const Model &orig = *GetOriginalModel();
    const TriMesh mesh = ScaleMesh(orig.GetMesh(), orig.GetScale());
    return Beveler::ApplyBevel(mesh, bevel_);
}

void BeveledModel::SyncTransformsFromOriginal(const Model &original) {
    // Leave the scale alone.
    SetRotation(original.GetRotation());
    SetTranslation(original.GetTranslation());
}

void BeveledModel::SyncTransformsToOriginal(Model &original) const {
    // Leave the scale alone.
    original.SetRotation(GetRotation());
    original.SetTranslation(GetTranslation());
}

void BeveledModel::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    // Copy the Bevel and original scale first.
    const BeveledModel &from_bev = static_cast<const BeveledModel &>(from);
    bevel_          = from_bev.bevel_;
    original_scale_ = from_bev.original_scale_;

    ConvertedModel::CopyContentsFrom(from, is_deep);
}

bool BeveledModel::ProcessChange(SG::Change change, const Object &obj) {
    if (! ConvertedModel::ProcessChange(change, obj))
        return false;

    // If the scale in the original changed, need to rebuild the mesh. Do NOT
    // do anything if there is no original model, which can happen during
    // CopyContentsFrom().
    const auto &orig = GetOriginalModel();
    if (orig && change == SG::Change::kTransform) {
        const Vector3f new_scale = orig->GetScale();
        if (new_scale != original_scale_) {
            original_scale_ = new_scale;
            MarkMeshAsStale();
        }
    }
    return true;
}
