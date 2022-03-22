#include "Commands/ChangeOrderCommand.h"

void ChangeOrderCommand::AddFields() {
    AddField(is_previous_);
    SingleModelCommand::AddFields();
}

std::string ChangeOrderCommand::GetDescription() const {
    return "Moved model " + GetModelDesc(GetModelName()) +
        (IsPrevious() ? " earlier" : " later") + " in the order";
}
