#include "Commands/LinearLayoutCommand.h"

#include "Targets/EdgeTarget.h"

void LinearLayoutCommand::AddFields() {
    AddField(offset_.Init("offset", Vector3f::Zero()));

    MultiModelCommand::AddFields();
}

std::string LinearLayoutCommand::GetDescription() const {
    return "Linear layout of " + GetModelsDesc(GetModelNames());
}

void LinearLayoutCommand::SetFromTarget(const EdgeTarget &target) {
    offset_ = target.GetLength() * target.GetDirection();
}
