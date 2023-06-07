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
            // Convert the Spin from stage to object coordinates.
            const auto som =
                pm.path_to_model.GetCoordConv().GetRootToObjectMatrix();
            const Spin object_spin = TransformSpin(csc.GetSpin(), som);

            // Save the current translation without offset compensation.
            const Vector3f trans =
                model.GetTranslation() - model.GetLocalCenterOffset();

            // Update the Spin in the Model.
            SetModelSpin(model, object_spin);

            // Compensate for any new centering translation.
            model.SetTranslation(trans + model.GetLocalCenterOffset());
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
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
