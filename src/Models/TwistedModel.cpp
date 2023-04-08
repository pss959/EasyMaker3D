#include "Models/TwistedModel.h"

#include <ion/math/vectorutils.h>

#include "Math/MeshUtils.h"
#include "Util/Assert.h"

void TwistedModel::AddFields() {
    AddModelField(center_.Init("center", Point3f::Zero()));
    AddModelField(axis_.Init("axis", Vector3f::AxisY()));
    AddModelField(angle_.Init("angle"));

    ConvertedModel::AddFields();
}

void TwistedModel::SetTwist(const Twist &twist) {
    center_ = twist.center;
    axis_   = twist.axis;
    angle_  = twist.angle;
    ProcessChange(SG::Change::kGeometry, *this);
}

TwistedModel::Twist TwistedModel::GetTwist() const {
    Twist twist;
    twist.center = center_;
    twist.axis   = axis_;
    twist.angle  = angle_;
    return twist;
}

TriMesh TwistedModel::BuildMesh() {
    // Twist the untransformed original mesh.
    ASSERT(GetOriginalModel());
    TriMesh mesh = GetOriginalModel()->GetMesh();
    // XXXX DO THE TWIST.
    return mesh;
}
