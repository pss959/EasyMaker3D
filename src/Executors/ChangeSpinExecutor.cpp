//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ChangeSpinExecutor.h"

#include "Commands/ChangeSpinCommand.h"
#include "Managers/SelectionManager.h"
#include "Math/Linear.h"
#include "SG/CoordConv.h"

void ChangeSpinExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeSpinCommand &csc = GetTypedCommand<ChangeSpinCommand>(command);
    for (auto &pm: data.per_model) {
        auto &model = *pm.path_to_model.GetModel();
        if (operation == Command::Op::kDo) {
            // Convert the Spin from stage to object coordinates of the
            // ConvertedModel, disregarding the current offset.
            const auto som =
                pm.path_to_model.GetCoordConv().GetRootToObjectMatrix();
            Spin spin = TransformSpin(csc.GetSpin(), som);
            spin.center += model.GetLocalCenterOffset();

            // Update the Spin in the Model and compensate for any new offset.
            SetModelSpin(model, spin);
            model.SetTranslation(pm.base_translation +
                                 model.GetLocalCenterOffset());
        }
        else {
            SetModelSpin(model, pm.old_spin);
            model.SetTranslation(pm.old_translation);
        }
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ChangeSpinExecutor::ExecData_ & ChangeSpinExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeSpinCommand &csc = GetTypedCommand<ChangeSpinCommand>(command);

        const auto &model_names = csc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());

        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm   = data->per_model[i];
            const SelPath path = FindPathToModel(model_names[i]);
            auto &model        = *path.GetModel();
            pm.path_to_model   = path;
            pm.old_spin        = GetModelSpin(model);
            pm.old_translation = model.GetTranslation();

            // Save the base translation, which has no offset.
            pm.base_translation =
                pm.old_translation - model.GetLocalCenterOffset();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
