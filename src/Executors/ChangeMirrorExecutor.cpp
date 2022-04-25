#include "Executors/ChangeMirrorExecutor.h"

#include "Commands/ChangeMirrorCommand.h"
#include "Managers/SelectionManager.h"
#include "Math/Linear.h"
#include "Models/MirroredModel.h"

void ChangeMirrorExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeMirrorCommand &ccc = GetTypedCommand<ChangeMirrorCommand>(command);

    if (operation == Command::Op::kDo) {
        for (auto &path: data.paths_to_models) {
            // Convert the plane from stage into local coordinates.
            const Matrix4f sol = CoordConv(path).GetRootToLocalMatrix();
            MirroredModel &cm = GetTypedModel<MirroredModel>(path);
            cm.AddPlane(TransformPlane(ccc.GetPlane(), sol));
        }
    }
    else {  // Undo.
        for (auto &path: data.paths_to_models) {
            MirroredModel &cm = GetTypedModel<MirroredModel>(path);
            cm.RemoveLastPlane();
        }
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ChangeMirrorExecutor::ExecData_ & ChangeMirrorExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeMirrorCommand &ccc = GetTypedCommand<ChangeMirrorCommand>(command);

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
