#include "Executors/ChangeClipExecutor.h"

#include "Commands/ChangeClipCommand.h"
#include "Managers/SelectionManager.h"
#include "Math/Linear.h"
#include "Models/ClippedModel.h"

void ChangeClipExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    if (operation == Command::Op::kDo) {
        ChangeClipCommand &ccc = GetTypedCommand<ChangeClipCommand>(command);
        for (auto &pm: data.per_model) {
            ClippedModel &cm = GetTypedModel<ClippedModel>(pm.path_to_model);
            // Save the current translation without offset compensation.
            const Vector3f trans =
                cm.GetTranslation() - cm.GetLocalCenterOffset();

            // Convert the plane from stage coordinates into object coordinates.
            const Plane object_plane = TransformPlane(
                TranslatePlane(ccc.GetPlane(), cm.GetLocalCenterOffset()),
                SG::CoordConv(pm.path_to_model).GetRootToObjectMatrix());

            // Set the plane in the ClippedModel and compensate for any new
            // centering translation.
            cm.SetPlane(object_plane);
            cm.SetTranslation(trans + cm.GetLocalCenterOffset());
        }
    }
    else {
        for (auto &pm: data.per_model) {
            ClippedModel &cm = GetTypedModel<ClippedModel>(pm.path_to_model);
            cm.SetPlane(pm.old_object_plane);
            cm.SetTranslation(pm.old_translation);
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
            ExecData_::PerModel &pm   = data->per_model[i];
            const SelPath        path = FindPathToModel(model_names[i]);
            const ClippedModel  &cm   = GetTypedModel<ClippedModel>(path);
            pm.path_to_model    = path;
            pm.old_object_plane = cm.GetPlane();

            pm.old_translation  = cm.GetTranslation();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
