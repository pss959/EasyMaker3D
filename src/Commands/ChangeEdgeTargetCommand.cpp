//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeEdgeTargetCommand.h"

#include "Parser/Registry.h"

void ChangeEdgeTargetCommand::AddFields() {
    AddField(old_target_.Init("old_target"));
    AddField(new_target_.Init("new_target"));

    Command::AddFields();
}

bool ChangeEdgeTargetCommand::IsValid(Str &details) {
    if (! Command::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    if (! old_target_.GetValue()) {
        details = "Missing old target";
        return false;
    }
    if (! new_target_.GetValue()) {
        details = "Missing new target";
        return false;
    }
    return true;
}

void ChangeEdgeTargetCommand::CreationDone() {
    Command::CreationDone();
    if (! IsTemplate()) {
        auto create_target = [](Parser::ObjectField<EdgeTarget> &field){
            if (! field.GetValue())
                field = Parser::Registry::CreateObject<EdgeTarget>();
        };

        // Make sure both old and new targets exist. They may be null if this
        // instance was created by the Parser::Registry.
        create_target(old_target_);
        create_target(new_target_);
    }
}

Str ChangeEdgeTargetCommand::GetDescription() const {
    return "Changed the edge target";
}

void ChangeEdgeTargetCommand::SetOldTarget(const EdgeTarget &target) {
    ASSERT(old_target_.GetValue());
    old_target_.GetValue()->CopyFrom(target);
}

void ChangeEdgeTargetCommand::SetNewTarget(const EdgeTarget &target) {
    ASSERT(new_target_.GetValue());
    new_target_.GetValue()->CopyFrom(target);
}
