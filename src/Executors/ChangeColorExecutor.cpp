//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ChangeColorExecutor.h"

#include "Commands/ChangeColorCommand.h"
#include "Managers/SelectionManager.h"
#include "Models/Model.h"
#include "Util/Assert.h"

void ChangeColorExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    if (operation == Command::Op::kDo) {
        ChangeColorCommand &ccc = GetTypedCommand<ChangeColorCommand>(command);
        for (auto &pm: data.per_model)
            pm.path_to_model.GetModel()->SetColor(ccc.GetNewColor());
    }
    else {  // Undo.
        for (auto &pm: data.per_model)
            pm.path_to_model.GetModel()->SetColor(pm.old_color);
    }

    // Reselect if undo or if command is finished being done. This is necessary
    // to update the current color in the ColorTool.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ChangeColorExecutor::ExecData_ & ChangeColorExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeColorCommand &ccc = GetTypedCommand<ChangeColorCommand>(command);

        const auto &model_names = ccc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model = FindPathToModel(model_names[i]);
            pm.old_color     = pm.path_to_model.GetModel()->GetColor();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
