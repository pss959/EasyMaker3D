#include "Models/TaperedModel.h"

#include "Math/Linear.h"
#include "Math/MeshDividing.h"
#include "Math/MeshUtils.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

void TaperedModel::AddFields() {
    AddModelField(axis_.Init("axis", Axis::kY));
    AddModelField(profile_points_.Init("profile_points"));

    ConvertedModel::AddFields();
}

bool TaperedModel::IsValid(std::string &details) {
    if (! ConvertedModel::IsValid(details))
        return false;

    // Construct and validate the Profile if points were specified.
    if (profile_points_.WasSet()) {
        if (! Taper::IsValidProfile(CreateProfile(profile_points_))) {
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

        prev_taper_ = taper_;
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
    // Use the previous Taper to determine what has changed.
    const int changed_index = GetChangedProfileIndex_();

    const int dim = Util::EnumInt(taper_.axis);
    const Vector3f dir = GetAxis(dim);

    // -1 means that the axis, the number of points, or more than 1 point
    // changed. In this case, need to re-slice from scratch.  Otherwise, just
    // add a new slice at the changed point and retaper.
    TriMesh new_mesh;
    if (changed_index < 0) {
        const std::vector<float> fractions =
            Util::ConvertVector<float, Point2f>(taper_.profile.GetPoints(),
                                                [&](const Point2f &p){
                                                    return p[1];
                                                });
        sliced_mesh_ = SliceMesh(mesh, dir, fractions);
    }
    else {
        // Slice the mesh at the new profile point location if it changed
        // enough in Y, unless it is an end points. Note that this results in
        // more slices than are needed in some cases, but it's much faster than
        // starting from scratch each time.
        const auto &prof_pts = taper_.profile.GetPoints();
        const size_t index = static_cast<size_t>(changed_index);
        if (prof_pts.size() > 2U && index > 0U && index + 1 < prof_pts.size()) {
            const float old_y = prev_taper_.profile.GetPoints()[index][1];
            const float new_y = prof_pts[index][1];
            if (std::abs(old_y - new_y) > TK::kMinTaperProfileYDistance) {
                const std::vector<float> fractions(1, prof_pts[index][1]);
                sliced_mesh_ = SliceMesh(sliced_mesh_.mesh, sliced_mesh_.dir,
                                         fractions);
            }
        }
    }
    prev_taper_ = taper_;
    return TaperMesh(sliced_mesh_, taper_);
}

int TaperedModel::GetChangedProfileIndex_() const {
    int index = -1;
    const auto &old_pts = prev_taper_.profile.GetPoints();
    const auto &new_pts = taper_.profile.GetPoints();
    if (taper_.axis == prev_taper_.axis && old_pts.size() == new_pts.size()) {
        for (size_t i = 0; i < old_pts.size(); ++i) {
            if (new_pts[i] != old_pts[i]) {
                if (index >= 0) {
                    // More than 1 point changed.
                    index = -1;
                    break;
                }
                index = i;
            }
        }
    }
    return index;
}
