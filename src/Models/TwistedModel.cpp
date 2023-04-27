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
    if (complexity != split_complexity_ || twist_.axis != split_axis_) {
        split_complexity_ = complexity;
        split_axis_       = twist_.axis;
        const int num_slices = LerpInt(complexity, 1, 20);
        split_mesh_ = SliceMesh(mesh, num_slices, twist_.axis);
    }

    return TwistMesh(split_mesh_, twist_);
}
