#include "Executors/ChangeClipExecutor.h"

#include "Commands/ChangeClipCommand.h"
#include "Managers/SelectionManager.h"
#include "Math/Linear.h"
#include "Models/ClippedModel.h"

void ChangeClipExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeClipCommand &ccc = GetTypedCommand<ChangeClipCommand>(command);

    if (operation == Command::Op::kDo) {
        for (auto &path: data.paths_to_models) {
            // Convert the plane from stage into local coordinates.
            const Matrix4f sol = CoordConv(path).GetRootToLocalMatrix();
            ClippedModel &cm = GetTypedModel<ClippedModel>(path);
            cm.AddPlane(TransformPlane(ccc.GetPlane(), sol));
        }
    }
    else {  // Undo.
        for (auto &path: data.paths_to_models) {
            ClippedModel &cm = GetTypedModel<ClippedModel>(path);
            cm.RemoveLastPlane();
        }
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ChangeClipExecutor::ExecData_ & ChangeClipExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeClipCommand &ccc = GetTypedCommand<ChangeClipCommand>(command);

        const auto &model_names = ccc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->paths_to_models.reserve(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i)
            data->paths_to_models.push_back(FindPathToModel(model_names[i]));
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
