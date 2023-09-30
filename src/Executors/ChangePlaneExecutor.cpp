#include "Executors/ChangePlaneExecutor.h"

#include "Commands/ChangePlaneCommand.h"
#include "Managers/SelectionManager.h"
#include "Math/Linear.h"
#include "Models/Model.h"

void ChangePlaneExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangePlaneCommand &cpc = GetTypedCommand<ChangePlaneCommand>(command);
    for (auto &pm: data.per_model) {
        auto &model = *pm.path_to_model.GetModel();
        if (operation == Command::Op::kDo) {
            // Let the derived class update the plane and whatever else is
            // necessary.
            const auto cc = pm.path_to_model.GetCoordConv();
            PlaneData data(cc);
            data.old_object_plane = pm.old_object_plane;
            data.new_stage_plane  = cpc.GetPlane();
            data.old_translation  = pm.old_translation;
            UpdateModel(model, data);
        }
        else {
            SetModelPlane(model, pm.old_object_plane);
            model.SetTranslation(pm.old_translation);
        }
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ChangePlaneExecutor::ExecData_ & ChangePlaneExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangePlaneCommand &cpc = GetTypedCommand<ChangePlaneCommand>(command);

        const auto &model_names = cpc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());

        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm   = data->per_model[i];
            const SelPath        path = FindPathToModel(model_names[i]);
            auto &model         = *path.GetModel();
            pm.path_to_model    = path;
            pm.old_object_plane = GetModelPlane(model);
            pm.old_translation  = model.GetTranslation();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
