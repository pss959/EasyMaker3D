#include "Models/BentModel.h"

#include "Math/Linear.h"
#include "Math/MeshSlicing.h"
#include "Math/MeshUtils.h"

TriMesh BentModel::ConvertMesh(const TriMesh &mesh) {
    const Spin spin = GetSpin();

    // Reslice if the complexity or axis changed.
    const float complexity = GetComplexity();
    if (complexity != sliced_complexity_ || spin.axis != sliced_axis_) {
        sliced_complexity_ = complexity;
        sliced_axis_       = spin.axis;
        const size_t num_slices = LerpInt(complexity, 1, 60);

        // Use the scaled operand mesh.
        const TriMesh scaled_mesh =
            ScaleMesh(mesh, GetOperandModel()->GetScale());

        // Figure out which dimension to use for slicing and slice the mesh.
        const Dim slice_dim = GetSliceDim_(scaled_mesh, spin.axis);
        sliced_mesh_ = SliceMesh(scaled_mesh, slice_dim, num_slices);
    }

    return BendMesh(sliced_mesh_, spin);
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
