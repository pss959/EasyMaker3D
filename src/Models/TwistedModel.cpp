#include "Models/TwistedModel.h"

#include "Math/Linear.h"
#include "Math/MeshSlicing.h"
#include "Math/MeshUtils.h"
#include "Util/Assert.h"

void TwistedModel::AddFields() {
    Spin default_spin;
    AddModelField(center_.Init("center", default_spin.center));
    AddModelField(axis_.Init("axis",     default_spin.axis));
    AddModelField(angle_.Init("angle",   default_spin.angle));

    ConvertedModel::AddFields();
}

bool TwistedModel::IsValid(std::string &details) {
    if (! ConvertedModel::IsValid(details))
        return false;
    if (! IsValidVector(axis_)) {
        details = "zero-length twist spin axis";
        return false;
    }
    return true;
}

void TwistedModel::CreationDone() {
    ConvertedModel::CreationDone();

    if (! IsTemplate()) {
        spin_.center = center_;
        spin_.axis   = axis_;
        spin_.angle  = angle_;
    }
}

void TwistedModel::SetSpin(const Spin &spin) {
    ASSERT(IsValidVector(spin.axis));
    spin_  = spin;
    center_ = spin.center;
    axis_   = spin.axis;
    angle_  = spin.angle;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh TwistedModel::ConvertMesh(const TriMesh &mesh) {
    // Reslice if the complexity or axis changed.
    const float complexity = GetComplexity();
    if (complexity != sliced_complexity_ || spin_.axis != sliced_axis_) {
        sliced_complexity_ = complexity;
        sliced_axis_       = spin_.axis;
        const size_t num_slices = LerpInt(complexity, 1, 20);

        if (spin_.axis == Vector3f::AxisY()) {
            sliced_mesh_ = SliceMesh(mesh, Dim::kY, num_slices);
        }
        else {
            // If the spin axis is not the +Y axis, rotate the mesh so it is,
            // apply the spin, and rotate back.
            const Rotationf rot =
                Rotationf::RotateInto(spin_.axis, Vector3f::AxisY());

            sliced_mesh_ = SliceMesh(RotateMesh(mesh, rot), Dim::kY,
                                     num_slices);

            sliced_mesh_.mesh = RotateMesh(sliced_mesh_.mesh, -rot);
        }
    }

    return TwistMesh(sliced_mesh_, spin_);
}
