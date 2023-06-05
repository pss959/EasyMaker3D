#include "Models/TwistedModel.h"

#include "Math/Linear.h"
#include "Math/MeshSlicing.h"
#include "Math/MeshUtils.h"

TriMesh TwistedModel::ConvertMesh(const TriMesh &mesh) {
    const Spin spin = GetSpin();

    // Reslice if the complexity or axis changed.
    const float complexity = GetComplexity();
    if (complexity != sliced_complexity_ || spin.axis != sliced_axis_) {
        sliced_complexity_ = complexity;
        sliced_axis_       = spin.axis;
        const size_t num_slices = LerpInt(complexity, 1, 20);

        // Use the scaled operand mesh.
        const TriMesh scaled_mesh =
            ScaleMesh(mesh, GetOperandModel()->GetScale());

        if (spin.axis == Vector3f::AxisY()) {
            sliced_mesh_ = SliceMesh(scaled_mesh, Dim::kY, num_slices);
        }
        else {
            // If the spin axis is not the +Y axis, rotate the mesh so it is,
            // apply the spin, and rotate back.
            const Rotationf rot =
                Rotationf::RotateInto(spin.axis, Vector3f::AxisY());

            sliced_mesh_ = SliceMesh(RotateMesh(scaled_mesh, rot), Dim::kY,
                                     num_slices);

            sliced_mesh_.mesh = RotateMesh(sliced_mesh_.mesh, -rot);
        }
    }

    return TwistMesh(sliced_mesh_, spin);
}
