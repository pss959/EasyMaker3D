#include "Models/BeveledModel.h"

#include "Math/CGALInterface.h"
#include "SG/Exception.h"
#include "Util/String.h"

void BeveledModel::AddFields() {
    AddField(profile_);
    AddField(bevel_scale_);
    AddField(max_angle_);
    ConvertedModel::AddFields();
}

void BeveledModel::AllFieldsParsed() {
    ConvertedModel::AllFieldsParsed();
    if (! profile_.GetValue().IsValid(0))
        ThrowReadError("Invalid profile");
    if (bevel_scale_ <= 0)
        ThrowReadError("Non-positive scale value");

    bevel_.profile   = profile_;
    bevel_.scale     = bevel_scale_;
    bevel_.max_angle = max_angle_;
}

void BeveledModel::SetBevel(const Bevel &bevel) {
    bevel_       = bevel;
    profile_     = bevel_.profile;
    bevel_scale_ = bevel_.scale;
    max_angle_   = bevel_.max_angle;
    ProcessChange(SG::Change::kGeometry);
}

TriMesh BeveledModel::BuildMesh() {
    TriMesh mesh;
    // XXXX Do something.
    return GetOriginalModel()->GetMesh();
}
