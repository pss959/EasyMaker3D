#include "Commands/ChangeClipCommand.h"

#include <ion/math/vectorutils.h>

#include "Parser/Registry.h"
#include "Util/Assert.h"

void ChangeClipCommand::AddFields() {
    AddField(plane_.Init("plane"));
    MultiModelCommand::AddFields();
}

bool ChangeClipCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (ion::math::Length(GetPlane().normal) < .00001f) {
        details = "zero-length plane normal";
        return false;
    }
    return true;
}

std::string ChangeClipCommand::GetDescription() const {
    return "Clipped " + GetModelsDesc(GetModelNames()) + " by plane " +
        GetPlane().ToString();
}

void ChangeClipCommand::SetPlane(const Plane &plane) {
    ASSERT(ion::math::Length(plane.normal) >= .00001f);
    plane_ = plane;
}
