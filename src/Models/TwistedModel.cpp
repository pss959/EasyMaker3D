#include "Models/TwistedModel.h"

#include "Math/Linear.h"
#include "Util/Assert.h"

void TwistedModel::AddFields() {
    AddModelField(center_.Init("center", Point3f::Zero()));
    AddModelField(axis_.Init("axis", Vector3f::AxisY()));
    AddModelField(angle_.Init("angle"));

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

void TwistedModel::SetTwist(const Twist &twist) {
    ASSERT(IsValidVector(twist.axis));
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

TriMesh TwistedModel::ConvertMesh(const TriMesh &mesh) {
    // XXXX DO THE TWIST.
    return mesh;
}
