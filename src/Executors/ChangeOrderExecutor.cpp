//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ChangeOrderExecutor.h"

#include "Commands/ChangeOrderCommand.h"
#include "Managers/SelectionManager.h"
#include "Models/ParentModel.h"

void ChangeOrderExecutor::Execute(Command &command, Command::Op operation) {
    ChangeOrderCommand &coc = GetTypedCommand<ChangeOrderCommand>(command);

    // Get the selected Model and its parent.
    const SelPath path = FindPathToModel(coc.GetModelName());
    const auto &model  = path.GetModel();
    const auto &parent = path.GetParentModel();

    // Save the current Selection so it can be restored.
    const Selection sel(path);

    // Determine the direction to change the order.
    const bool is_previous =
        (operation == Command::Op::kDo   &&   coc.IsPrevious()) ||
        (operation == Command::Op::kUndo && ! coc.IsPrevious());

    const int index = parent->GetChildModelIndex(model);
    ASSERT(index >= 0);
    parent->RemoveChildModel(index);
    parent->InsertChildModel(is_previous ? index - 1 : index + 1, model);

    // Can't just reselect, since the moved Model's status may have changed, so
    // change the selection explicitly.
    GetContext().selection_manager->ChangeSelection(sel);
}

