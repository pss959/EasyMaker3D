#include "Commands/ChangeMirrorCommand.h"

#include <ion/math/vectorutils.h>

#include "Parser/Registry.h"
#include "Util/Assert.h"

void ChangeMirrorCommand::AddFields() {
    AddField(plane_);
    MultiModelCommand::AddFields();
}

bool ChangeMirrorCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (ion::math::Length(GetPlane().normal) < .00001f) {
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
    ASSERT(ion::math::Length(plane.normal) >= .00001f);
    plane_ = plane;
}
