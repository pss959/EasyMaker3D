//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Models/RevSurfModel.h"

#include "Math/Linear.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

void RevSurfModel::AddFields() {
    AddModelField(profile_points_.Init("profile_points"));
    AddModelField(sweep_angle_.Init("sweep_angle", Anglef::FromDegrees(360)));

    PrimitiveModel::AddFields();
}

bool RevSurfModel::IsValid(Str &details) {
    if (! PrimitiveModel::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    // Construct and validate the Profile if points were specified.
    if (profile_points_.WasSet()) {
        if (! CreateProfile(profile_points_).IsValid()) {
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
    PrimitiveModel::CreationDone();

    if (! IsTemplate()) {
        if (profile_points_.WasSet()) {
            profile_ = CreateProfile(profile_points_);
            ASSERT(profile_.IsValid());
        }
    }
}

void RevSurfModel::SetProfile(const Profile &profile) {
    ASSERT(profile.IsValid());
    profile_ = profile;
    profile_points_ = profile_.GetPoints();
    ProcessChange(SG::Change::kGeometry, *this);
}

void RevSurfModel::SetSweepAngle(const Anglef &angle) {
    ASSERT(angle.Degrees() > 0 && angle.Degrees() <= 360);
    sweep_angle_ = angle;
    ProcessChange(SG::Change::kGeometry, *this);
}

Profile RevSurfModel::CreateProfile(const Profile::PointVec &points) {
    return Profile::CreateFixedProfile(Point2f(0, 1), Point2f(0, 0), 3, points);
}

Profile RevSurfModel::CreateDefaultProfile() {
    return CreateProfile(Profile::PointVec{ Point2f(.5f, .5f) });
}

TriMesh RevSurfModel::BuildMesh() {
    // Determine the number of sides based on the complexity.
    const int num_sides = LerpInt(GetComplexity(), 3, TK::kMaxRevSurfSides);
    const TriMesh mesh = BuildRevSurfMesh(profile_, sweep_angle_, num_sides);

    // If not a full sweep, center the mesh and save the offset.
    const float angle = GetSweepAngle().Degrees();
    if (angle < 360) {
        SetCenterOffset(Vector3f(ComputeMeshBounds(mesh).GetCenter()));
        return CenterMesh(mesh);
    }
    else {
        SetCenterOffset(Vector3f::Zero());
        return mesh;
    }
}
