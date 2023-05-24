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

        // Figure out which axis to use for slicing and slice the mesh.
        const Axis slice_axis = GetSliceAxis_(scaled_mesh, bend_.axis);
        sliced_mesh_ = SliceMesh(scaled_mesh, slice_axis, num_slices);
    }

    return BendMesh(sliced_mesh_, bend_);
}

Axis BentModel::GetSliceAxis_(const TriMesh &mesh, const Vector3f &bend_axis) {
    // Use the longer of the two principal axes that are more perpendicular to
    // the bend axis.
    const int bend_axis_dim = GetMaxAbsElementIndex(bend_axis);
    const int dim0 = (bend_axis_dim + 1) % 3;
    const int dim1 = (bend_axis_dim + 2) % 3;

    const auto scaled_size = ComputeMeshBounds(mesh).GetSize();
    const int slice_dim = scaled_size[dim0] >= scaled_size[dim1] ? dim0 : dim1;

    return static_cast<Axis>(slice_dim);
}
