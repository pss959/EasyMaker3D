#include "Commands/ChangePointTargetCommand.h"

void ChangePointTargetCommand::AddFields() {
    AddField(old_target_);
    AddField(new_target_);
    Command::AddFields();
}

std::string ChangePointTargetCommand::GetDescription() const {
    return "Changed the point target";
}
