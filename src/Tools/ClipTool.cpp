#include "Tools/ClipTool.h"

#include <algorithm>
#include <limits>

#include <ion/math/transformutils.h>

#include "Commands/ChangeClipCommand.h"
#include "Math/Linear.h"
#include "Models/ClippedModel.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

ClipTool::ClipTool() {
}

bool ClipTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<ClippedModel>(sel);
}

Plane ClipTool::GetObjectPlaneFromModel() const {
    auto cm = std::dynamic_pointer_cast<ClippedModel>(GetModelAttachedTo());
    ASSERT(cm);
    return cm->GetPlane();
}

Range1f ClipTool::GetTranslationRange() const {
    // Set the range, making sure not to clip away all of the mesh by
    // restricting the minimum and maximum values.
    Range1f distance_range;
    Range3f point_range;
    GetRangeAlongPlaneNormal_(distance_range, point_range);
    return Range1f(distance_range.GetMinPoint() + TK::kMinClippedSize,
                   distance_range.GetMaxPoint() - TK::kMinClippedSize);
}

ChangePlaneCommandPtr ClipTool::CreateChangePlaneCommand() const {
    return CreateCommand<ChangeClipCommand>();
}

Point3f ClipTool::GetTranslationFeedbackBasePoint() const {
    // Use the minimum point (relative to the plane) in stage coordinates.
    Range1f distance_range;
    Range3f point_range;
    GetRangeAlongPlaneNormal_(distance_range, point_range);

    auto cm = std::dynamic_pointer_cast<ClippedModel>(GetModelAttachedTo());
    ASSERT(cm);
    return GetModelMatrix() * point_range.GetMinPoint();
}

void ClipTool::GetRangeAlongPlaneNormal_(Range1f &distance_range,
                                         Range3f &point_range) const {
    // Compute the min/max signed distances of any vertex of the unclipped mesh
    // along the clipping plane's normal vector. This assumes that the
    // PlaneWidget is centered on the unclipped mesh so that the mesh's center
    // point is at a distance of 0. Note that the mesh points need to be scaled
    // by the ClippedModel's scale to bring them into the object coordinates of
    // the ClipTool.
    auto cm = std::dynamic_pointer_cast<ClippedModel>(GetModelAttachedTo());
    ASSERT(cm);
    const auto &mesh      = cm->GetOperandModel()->GetMesh();
    const Vector3f &scale = cm->GetScale();
    float min_dist =  std::numeric_limits<float>::max();
    float max_dist = -std::numeric_limits<float>::max();
    const Plane object_plane = cm->GetPlane();
    Point3f min_pt(0, 0, 0);
    Point3f max_pt(0, 0, 0);
    for (const Point3f &p: mesh.points) {
        const auto scaled_pt = ScalePoint(p, scale);
        const float dist = SignedDistance(scaled_pt, object_plane.normal);
        if (dist < min_dist) {
            min_dist = dist;
            min_pt   = scaled_pt;
        }
        if (dist > max_dist) {
            max_dist = dist;
            max_pt   = scaled_pt;
        }
    }
    distance_range.Set(min_dist, max_dist);
    point_range.Set(min_pt, max_pt);
}
