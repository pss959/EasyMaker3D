#include "Executors/ChangeClipExecutor.h"

#include <ion/math/transformutils.h>

#include "Commands/ChangeClipCommand.h"
#include "Managers/SelectionManager.h"
#include "Math/Linear.h"
#include "Models/ClippedModel.h"

void ChangeClipExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    const ChangeClipCommand &ccc = GetTypedCommand<ChangeClipCommand>(command);

    for (auto &pm: data.per_model) {
        ClippedModel &cm = GetTypedModel<ClippedModel>(pm.path_to_model);
        if (operation == Command::Op::kDo) {
            // Convert Plane from stage to object coordinates.
            const Matrix4f som =
                SG::CoordConv(pm.path_to_model).GetRootToObjectMatrix();
            cm.SetPlane(TransformPlane(ccc.GetPlane(), som));
        }
        else {
            cm.SetPlane(pm.old_plane);
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
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model = FindPathToModel(model_names[i]);
            ClippedModel &cm = GetTypedModel<ClippedModel>(pm.path_to_model);
            pm.old_plane = cm.GetPlane();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
