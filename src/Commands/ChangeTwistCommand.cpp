#include "Commands/ChangeTwistCommand.h"

#include "Math/Linear.h"
#include "Parser/Registry.h"
#include "Util/Assert.h"

void ChangeTwistCommand::AddFields() {
    AddField(center_.Init("center", Point3f::Zero()));
    AddField(axis_.Init("axis", Vector3f::AxisY()));
    AddField(angle_.Init("angle"));

    MultiModelCommand::AddFields();
}

bool ChangeTwistCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (! IsValidVector(axis_)) {
         details = "zero-length axis";
        return false;
    }
    return true;
}

std::string ChangeTwistCommand::GetDescription() const {
    return "Twisted " + GetModelsDesc(GetModelNames());
}

void ChangeTwistCommand::SetTwist(const TwistedModel::Twist &twist) {
    center_ = twist.center;
    axis_   = twist.axis;
    angle_  = twist.angle;
}

TwistedModel::Twist ChangeTwistCommand::GetTwist() const {
    TwistedModel::Twist twist;
    twist.center = center_;
    twist.axis   = axis_;
    twist.angle  = angle_;
    return twist;
}
