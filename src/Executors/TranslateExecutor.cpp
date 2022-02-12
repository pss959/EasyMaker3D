#include "Executors/TranslateExecutor.h"

#include <ion/math/transformutils.h>

#include "Commands/TranslateCommand.h"

void TranslateExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    if (operation == Command::Op::kDo) {
        TranslateCommand &tc = GetTypedCommand<TranslateCommand>(command);
        TranslateModels_(data, tc.GetTranslation());
    }
    else {  // Undo.
        for (auto &pm: data.per_model)
            pm.path_to_model.GetModel()->SetTranslation(pm.old_translation);
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

TranslateExecutor::ExecData_ & TranslateExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        TranslateCommand &tc = GetTypedCommand<TranslateCommand>(command);

        const auto &model_names = tc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model   = FindPathToModel(model_names[i]);
            pm.old_translation = pm.path_to_model.GetModel()->GetTranslation();
            pm.new_translation = pm.old_translation;
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}

void TranslateExecutor::TranslateModels_(ExecData_ &data,
                                         const Vector3f &translation) {
    for (auto &pm: data.per_model) {
        // Convert the stage-space motion into local motion. (Local, not
        // object, since it needs to include the Model's scale and rotation.
        const auto &path = pm.path_to_model;
        pm.new_translation =
            pm.old_translation + path.GetStageToLocalMatrix() * translation;
        path.GetModel()->SetTranslation(pm.new_translation);
    }
}
