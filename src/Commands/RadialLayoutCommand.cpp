#include "Commands/RadialLayoutCommand.h"

#include "Place/PointTarget.h"

void RadialLayoutCommand::AddFields() {
    AddField(center_.Init("center", Point3f::Zero()));
    AddField(normal_.Init("normal", Vector3f::AxisY()));
    AddField(radius_.Init("radius", 1));
    AddField(arc_.Init("arc"));

    MultiModelCommand::AddFields();
}

Str RadialLayoutCommand::GetDescription() const {
    return "Radial layout of " + GetModelsDesc(GetModelNames());
}

void RadialLayoutCommand::SetFromTarget(const PointTarget &target) {
    center_ = target.GetPosition();
    normal_ = target.GetDirection();
    radius_ = target.GetRadius();
    arc_    = target.GetArc();
}
