#include "Commands/ChangeBendCommand.h"

#include "Math/Linear.h"
#include "Math/Bend.h"
#include "Parser/Registry.h"
#include "Util/Assert.h"

void ChangeBendCommand::AddFields() {
    AddField(center_.Init("center", Point3f::Zero()));
    AddField(axis_.Init("axis", Vector3f::AxisY()));
    AddField(angle_.Init("angle"));

    MultiModelCommand::AddFields();
}

bool ChangeBendCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (! IsValidVector(axis_)) {
         details = "zero-length axis";
        return false;
    }
    return true;
}

std::string ChangeBendCommand::GetDescription() const {
    return "Bent " + GetModelsDesc(GetModelNames());
}

void ChangeBendCommand::SetBend(const Bend &bend) {
    center_ = bend.center;
    axis_   = bend.axis;
    angle_  = bend.angle;
}

Bend ChangeBendCommand::GetBend() const {
    Bend bend;
    bend.center = center_;
    bend.axis   = axis_;
    bend.angle  = angle_;
    return bend;
}
