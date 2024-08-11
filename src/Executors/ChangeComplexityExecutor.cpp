//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ChangeComplexityExecutor.h"

#include "Commands/ChangeComplexityCommand.h"
#include "Managers/SelectionManager.h"

void ChangeComplexityExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    if (operation == Command::Op::kDo) {
        ChangeComplexityCommand &ccc =
            GetTypedCommand<ChangeComplexityCommand>(command);
        for (auto &pm: data.per_model)
            pm.path_to_model.GetModel()->SetComplexity(ccc.GetNewComplexity());
    }
    else {  // Undo.
        for (auto &pm: data.per_model)
            pm.path_to_model.GetModel()->SetComplexity(pm.old_complexity);
    }

    // Reselect if undo or if command is finished being done in case the size
    // changed.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ChangeComplexityExecutor::ExecData_ & ChangeComplexityExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeComplexityCommand &ccc =
            GetTypedCommand<ChangeComplexityCommand>(command);

        const auto &model_names = ccc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model  = FindPathToModel(model_names[i]);
            pm.old_complexity = pm.path_to_model.GetModel()->GetComplexity();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
