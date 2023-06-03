#include "Commands/ChangePlaneCommand.h"

#include "Math/Linear.h"
#include "Util/Assert.h"

void ChangePlaneCommand::AddFields() {
    AddField(plane_.Init("plane"));
    MultiModelCommand::AddFields();
}

bool ChangePlaneCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (! IsValidVector(GetPlane().normal)) {
        details = "zero-length plane normal";
        return false;
    }
    return true;
}

void ChangePlaneCommand::SetPlane(const Plane &plane) {
    ASSERT(IsValidVector(plane.normal));
    plane_ = plane;
}
