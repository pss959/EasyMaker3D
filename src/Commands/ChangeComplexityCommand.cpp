#include "Commands/ChangeComplexityCommand.h"

void ChangeComplexityCommand::AddFields() {
    AddField(new_complexity_.Init("new_complexity"));

    MultiModelCommand::AddFields();
}

std::string ChangeComplexityCommand::GetDescription() const {
    return "Changed the complexity of " + GetModelsDesc(GetModelNames());
}
