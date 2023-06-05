#include "Models/BentModel.h"

#include "Math/Linear.h"
#include "Math/MeshSlicing.h"
#include "Math/MeshUtils.h"
#include "Util/Assert.h"

void BentModel::AddFields() {
    Spin default_spin;
    AddModelField(center_.Init("center", default_spin.center));
    AddModelField(axis_.Init("axis",     default_spin.axis));
    AddModelField(angle_.Init("angle",   default_spin.angle));
    AddModelField(offset_.Init("offset", default_spin.offset));

    ScaledConvertedModel::AddFields();
}

bool BentModel::IsValid(std::string &details) {
    if (! ScaledConvertedModel::IsValid(details))
        return false;
    if (! IsValidVector(axis_)) {
        details = "zero-length bend spin axis";
        return false;
    }
    return true;
}

void BentModel::CreationDone() {
    ScaledConvertedModel::CreationDone();

    if (! IsTemplate()) {
        spin_.center = center_;
        spin_.axis   = axis_;
        spin_.angle  = angle_;
        spin_.offset = offset_;
    }
}

void BentModel::SetSpin(const Spin &spin) {
    ASSERT(IsValidVector(spin.axis));
    spin_    = spin;
    center_  = spin.center;
    axis_    = spin.axis;
    angle_   = spin.angle;
    offset_  = spin.offset;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh BentModel::ConvertMesh(const TriMesh &mesh) {
    // Reslice if the complexity or axis changed.
    const float complexity = GetComplexity();
    if (complexity != sliced_complexity_ || spin_.axis != sliced_axis_) {
        sliced_complexity_ = complexity;
        sliced_axis_       = spin_.axis;
        const size_t num_slices = LerpInt(complexity, 1, 60);

        // Use the scaled operand mesh.
        const TriMesh scaled_mesh =
            ScaleMesh(mesh, GetOperandModel()->GetScale());

        // Figure out which dimension to use for slicing and slice the mesh.
        const Dim slice_dim = GetSliceDim_(scaled_mesh, spin_.axis);
        sliced_mesh_ = SliceMesh(scaled_mesh, slice_dim, num_slices);
    }

    return BendMesh(sliced_mesh_, spin_);
}

Dim BentModel::GetSliceDim_(const TriMesh &mesh, const Vector3f &spin_axis) {
    // Use the longer of the two principal axes that are more perpendicular to
    // the spin axis.
    const int spin_axis_dim = GetMaxAbsElementIndex(spin_axis);
    const int dim0 = (spin_axis_dim + 1) % 3;
    const int dim1 = (spin_axis_dim + 2) % 3;

    const auto scaled_size = ComputeMeshBounds(mesh).GetSize();
    const int slice_dim = scaled_size[dim0] >= scaled_size[dim1] ? dim0 : dim1;

    return static_cast<Dim>(slice_dim);
}
