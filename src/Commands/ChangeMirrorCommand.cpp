#include "Commands/ChangeMirrorCommand.h"

#include "Math/Linear.h"
#include "Parser/Registry.h"
#include "Util/Assert.h"

void ChangeMirrorCommand::AddFields() {
    AddField(plane_.Init("plane"));
    AddField(is_in_place_.Init("is_in_place", false));

    MultiModelCommand::AddFields();
}

bool ChangeMirrorCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (! IsValidVector(GetPlane().normal)) {
        details = "zero-length plane normal";
        return false;
    }
    return true;
}

std::string ChangeMirrorCommand::GetDescription() const {
    return "Mirrored " + GetModelsDesc(GetModelNames()) + " across plane " +
        GetPlane().ToString();
}

void ChangeMirrorCommand::SetPlane(const Plane &plane) {
    ASSERT(IsValidVector(plane.normal));
    plane_ = plane;
}
