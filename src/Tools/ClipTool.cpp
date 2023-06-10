#include "Tools/ClipTool.h"

#include <algorithm>
#include <limits>

#include <ion/math/transformutils.h>

#include "Commands/ChangeClipCommand.h"
#include "Math/Linear.h"
#include "Models/ClippedModel.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/Tuning.h"

ClipTool::ClipTool() {
}

bool ClipTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<ClippedModel>(sel);
}

Plane ClipTool::GetObjectPlaneFromModel() const {
    auto cm = Util::CastToDerived<ClippedModel>(GetModelAttachedTo());
    ASSERT(cm);
    return cm->GetPlane();
}

Range1f ClipTool::GetTranslationRange() const {
    // Compute the min/max signed distances of any vertex of the unclipped mesh
    // along the current clipping plane's normal vector. This assumes that the
    // PlaneWidget is centered on the unclipped mesh so that the mesh's center
    // point is at a distance of 0. Note that the mesh points need to be scaled
    // by the ClippedModel's scale to bring them into the object coordinates of
    // the ClipTool.
    auto cm = Util::CastToDerived<ClippedModel>(GetModelAttachedTo());
    ASSERT(cm);
    const auto &mesh        = cm->GetOperandModel()->GetMesh();
    const Vector3f &scale   = cm->GetScale();
    float min_dist =  std::numeric_limits<float>::max();
    float max_dist = -std::numeric_limits<float>::max();
    const Plane object_plane = cm->GetPlane();
    for (const Point3f &p: mesh.points) {
        const float dist = SignedDistance(ScalePoint(p, scale),
                                          object_plane.normal);
        min_dist = std::min(min_dist, dist);
        max_dist = std::max(max_dist, dist);
    }

    // Set the range, making sure not to clip away all of the mesh by
    // restricting the minimum and maximum values.
    return Range1f(min_dist + TK::kMinClippedSize,
                   max_dist - TK::kMinClippedSize);
}

ChangePlaneCommandPtr ClipTool::CreateChangePlaneCommand() const {
    return CreateCommand<ChangeClipCommand>();
}

Point3f ClipTool::GetTranslationFeedbackBasePoint() const {
    // Use the minimum point (relative to the plane) in stage coordinates.
    const auto &model = *GetModelAttachedTo();
    const auto &mesh        = model.GetMesh();
    const Vector3f &scale   = model.GetScale();
    float min_dist = std::numeric_limits<float>::max();
    const auto object_plane = GetObjectPlane();
    Point3f min_pt(0, 0, 0);
    for (const Point3f &p: mesh.points) {
        const Point3f scaled_pt = ScalePoint(p, scale);
        const float dist = SignedDistance(scaled_pt, object_plane.normal);
        if (dist < min_dist) {
            min_dist = dist;
            min_pt   = scaled_pt;
        }
    }
    return GetModelMatrix() * min_pt + model.GetLocalCenterOffset();
}
