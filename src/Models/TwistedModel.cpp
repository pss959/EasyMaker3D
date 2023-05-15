#include "Models/TwistedModel.h"

#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "Util/Assert.h"

void TwistedModel::AddFields() {
    Twist default_twist;
    AddModelField(center_.Init("center", default_twist.center));
    AddModelField(axis_.Init("axis",     default_twist.axis));
    AddModelField(angle_.Init("angle",   default_twist.angle));

    ConvertedModel::AddFields();
}

bool TwistedModel::IsValid(std::string &details) {
    if (! ConvertedModel::IsValid(details))
        return false;
    if (! IsValidVector(axis_)) {
        details = "zero-length twist axis";
        return false;
    }
    return true;
}

void TwistedModel::CreationDone() {
    ConvertedModel::CreationDone();

    if (! IsTemplate()) {
        twist_.center = center_;
        twist_.axis   = axis_;
        twist_.angle  = angle_;
    }
}

void TwistedModel::SetTwist(const Twist &twist) {
    ASSERT(IsValidVector(twist.axis));
    twist_  = twist;
    center_ = twist.center;
    axis_   = twist.axis;
    angle_  = twist.angle;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh TwistedModel::ConvertMesh(const TriMesh &mesh) {
    // Reslice if the complexity or axis changed.
    const float complexity = GetComplexity();
    if (complexity != sliced_complexity_ || twist_.axis != sliced_axis_) {
        sliced_complexity_ = complexity;
        sliced_axis_       = twist_.axis;
        const size_t num_slices = LerpInt(complexity, 1, 20);

        if (twist_.axis == Vector3f::AxisY()) {
            sliced_mesh_ = SliceMesh(mesh, Axis::kY, num_slices);
        }
        else {
            // If the twist axis is not the +Y axis, rotate the mesh so it is,
            // apply the twist, and rotate back.
            const Rotationf rot =
                Rotationf::RotateInto(twist_.axis, Vector3f::AxisY());

            sliced_mesh_ = SliceMesh(RotateMesh(mesh, rot), Axis::kY,
                                     num_slices);

            sliced_mesh_.mesh = RotateMesh(sliced_mesh_.mesh, -rot);
        }
    }

    return TwistMesh(sliced_mesh_, twist_);
}
