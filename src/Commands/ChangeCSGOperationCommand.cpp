#include "Commands/ChangeCSGOperationCommand.h"

void ChangeCSGOperationCommand::AddFields() {
    AddField(new_operation_);
    MultiModelCommand::AddFields();
}

std::string ChangeCSGOperationCommand::GetDescription() const {
    return "Change the CSG operation of " + GetModelsDesc(GetModelNames()) +
        " to " + new_operation_.GetEnumName();
}
