#include "Commands/ChangeTwistCommand.h"

#include <ion/math/vectorutils.h>

#include "Parser/Registry.h"
#include "Util/Assert.h"

void ChangeTwistCommand::AddFields() {
    AddField(plane_.Init("plane"));
    AddField(is_in_place_.Init("is_in_place", false));

    MultiModelCommand::AddFields();
}

bool ChangeTwistCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (ion::math::Length(GetPlane().normal) < .00001f) {
        details = "zero-length plane normal";
        return false;
    }
    return true;
}

std::string ChangeTwistCommand::GetDescription() const {
    return "Twisted " + GetModelsDesc(GetModelNames()) + " across plane " +
        GetPlane().ToString();
}

void ChangeTwistCommand::SetPlane(const Plane &plane) {
    ASSERT(ion::math::Length(plane.normal) >= .00001f);
    plane_ = plane;
}
