#include "Models/ExtrudedModel.h"

#include "Math/Curves.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Math/Polygon.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

void ExtrudedModel::AddFields() {
    AddModelField(profile_points_.Init("profile_points"));

    PrimitiveModel::AddFields();
}

bool ExtrudedModel::IsValid(std::string &details) {
    if (! PrimitiveModel::IsValid(details))
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

void ExtrudedModel::CreationDone() {
    PrimitiveModel::CreationDone();

    if (! IsTemplate()) {
        if (profile_points_.WasSet()) {
            profile_ = CreateProfile(profile_points_);
            ASSERT(profile_.IsValid());
        }
    }
}

void ExtrudedModel::SetProfile(const Profile &profile) {
    ASSERT(profile.IsValid());
    ASSERT(! profile.IsOpen());
    profile_ = profile;
    profile_points_ = profile_.GetPoints();
    ProcessChange(SG::Change::kGeometry, *this);
}

Profile ExtrudedModel::CreateRegularPolygonProfile(size_t side_count) {
    ASSERT(side_count >= 3U);

    /// XXXX Consider rotating based on odd/even number of points?

    // Generate points around a circle and offset them to put them in the range
    // (0,1).
    Profile::PointVec pts =
        GetCirclePoints(side_count, TK::kExtrudedModelRadius, false);

    for (auto &pt: pts)
        pt += Vector2f(.5f, .5f);

    return CreateProfile(pts);
}

Profile ExtrudedModel::CreateProfile(const Profile::PointVec &points) {
    ASSERT(points.size() >= 3U);
    return Profile(points, 3);
}

Profile ExtrudedModel::CreateDefaultProfile() {
    return CreateRegularPolygonProfile(4);
}

TriMesh ExtrudedModel::BuildMesh() {
    // Use a height of 1; the ExtrudedModel will be scaled when created.
    return CenterMesh(BuildExtrudedMesh(Polygon(profile_.GetPoints()), 1));
}
