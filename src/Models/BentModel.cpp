#include "Models/BentModel.h"

#include "Math/Linear.h"
#include "Math/MeshSlicing.h"
#include "Math/MeshUtils.h"
#include "Util/Assert.h"

void BentModel::AddFields() {
    Bend default_bend;
    AddModelField(center_.Init("center", default_bend.center));
    AddModelField(axis_.Init("axis",     default_bend.axis));
    AddModelField(angle_.Init("angle",   default_bend.angle));
    AddModelField(offset_.Init("offset", default_bend.offset));

    ScaledConvertedModel::AddFields();
}

bool BentModel::IsValid(std::string &details) {
    if (! ScaledConvertedModel::IsValid(details))
        return false;
    if (! IsValidVector(axis_)) {
        details = "zero-length bend axis";
        return false;
    }
    return true;
}

void BentModel::CreationDone() {
    ScaledConvertedModel::CreationDone();

    if (! IsTemplate()) {
        bend_.center = center_;
        bend_.axis   = axis_;
        bend_.angle  = angle_;
        bend_.offset = offset_;
    }
}

void BentModel::SetBend(const Bend &bend) {
    ASSERT(IsValidVector(bend.axis));
    bend_    = bend;
    center_  = bend.center;
    axis_    = bend.axis;
    angle_   = bend.angle;
    offset_  = bend.offset;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh BentModel::ConvertMesh(const TriMesh &mesh) {
    // Reslice if the complexity or axis changed.
    const float complexity = GetComplexity();
    if (complexity != sliced_complexity_ || bend_.axis != sliced_axis_) {
        sliced_complexity_ = complexity;
        sliced_axis_       = bend_.axis;
        const size_t num_slices = LerpInt(complexity, 1, 20);

        // Use the scaled operand mesh.
        const TriMesh scaled_mesh =
            ScaleMesh(mesh, GetOperandModel()->GetScale());

        // XXXX
        sliced_mesh_ = SliceMesh(scaled_mesh, Axis::kX, num_slices);

#if XXXX
        // Slice the mesh. Slice along the Y axis unless the bend axis is not
        // perpendicular to it.
        // XXXX Need to figure out how to get a real slicing axis here.
        if (bend_.axis[2] == 0) {
            sliced_mesh_ = SliceMesh(scaled_mesh, Axis::kY, num_slices);
        }
        else {
            // If the bend axis is not perpendicular to the +Y axis, rotate the
            // mesh to align with the +Z axis, apply the bend, and rotate back.
            const Rotationf rot =
                Rotationf::RotateInto(bend_.axis, Vector3f::AxisY());

            sliced_mesh_ = SliceMesh(RotateMesh(scaled_mesh, rot), Axis::kY,
                                     num_slices);

            sliced_mesh_.mesh = RotateMesh(sliced_mesh_.mesh, -rot);
        }
#endif
    }

    return BendMesh(sliced_mesh_, bend_);
}
