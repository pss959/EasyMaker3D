#include "Commands/ChangeClipCommand.h"

#include "Math/Linear.h"
#include "Parser/Registry.h"
#include "Util/Assert.h"

void ChangeClipCommand::AddFields() {
    AddField(plane_.Init("plane"));
    MultiModelCommand::AddFields();
}

bool ChangeClipCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (! IsValidVector(GetPlane().normal)) {
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
    ASSERT(IsValidVector(plane.normal));
    plane_ = plane;
}
