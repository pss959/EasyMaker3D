#include "Executors/ChangeClipExecutor.h"

#include <ion/math/transformutils.h>

#include "Commands/ChangeClipCommand.h"
#include "Managers/SelectionManager.h"
#include "Math/Linear.h"
#include "Models/ClippedModel.h"

void ChangeClipExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    for (auto &pm: data.per_model) {
        ClippedModel &cm = GetTypedModel<ClippedModel>(pm.path_to_model);
        if (operation == Command::Op::kDo)
            cm.AddPlane(pm.local_plane);
        else   // Undo.
            cm.RemoveLastPlane();
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
        data->per_model.resize(model_names.size());

        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            const SelPath path = FindPathToModel(model_names[i]);
            pm.path_to_model = path;

            // Convert plane from stage to local coordinates.
            const Matrix4f slm = CoordConv(path).GetRootToLocalMatrix();
            pm.local_plane = TransformPlane(ccc.GetPlane(), slm);
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
