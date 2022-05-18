#include "Commands/RadialLayoutCommand.h"

#include "Targets/PointTarget.h"

void RadialLayoutCommand::AddFields() {
    AddField(center_);
    AddField(normal_);
    AddField(radius_);
    AddField(start_angle_);
    AddField(arc_angle_);
    MultiModelCommand::AddFields();
}

std::string RadialLayoutCommand::GetDescription() const {
    return "Radial layout of " + GetModelsDesc(GetModelNames());
}

void RadialLayoutCommand::SetFromTarget(const PointTarget &target) {
    center_      = target.GetPosition();
    normal_      = target.GetDirection();
    radius_      = target.GetRadius();
    start_angle_ = target.GetStartAngle();
    arc_angle_   = target.GetArcAngle();
}
