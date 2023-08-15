#include "Commands/ChangeOrderCommand.h"

void ChangeOrderCommand::AddFields() {
    AddField(is_previous_.Init("is_previous", false));

    SingleModelCommand::AddFields();
}

std::string ChangeOrderCommand::GetDescription() const {
    return "Moved " + GetModelDesc(GetModelName()) +
        (IsPrevious() ? " earlier" : " later") + " in the order";
}
