#include "Executors/ChangeMirrorExecutor.h"

#include <ion/math/transformutils.h>

#include "Commands/ChangeMirrorCommand.h"
#include "Managers/SelectionManager.h"
#include "Math/Linear.h"
#include "Models/MirroredModel.h"

void ChangeMirrorExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    for (auto &pm: data.per_model) {
        MirroredModel &mm = GetTypedModel<MirroredModel>(pm.path_to_model);
        if (operation == Command::Op::kDo)
            mm.AddPlane(pm.local_plane);
        else   // Undo.
            mm.RemoveLastPlane();
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ChangeMirrorExecutor::ExecData_ & ChangeMirrorExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeMirrorCommand &cmc = GetTypedCommand<ChangeMirrorCommand>(command);

        const auto &model_names = cmc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());

        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            const SelPath path = FindPathToModel(model_names[i]);
            pm.path_to_model = path;

            // If operating in-place, use the local center of each Model for
            // the local plane. Otherwise, convert the plane from stage to
            // local coordinates.
            const Matrix4f slm = CoordConv(path).GetRootToLocalMatrix();
            pm.local_plane = TransformPlane(cmc.GetPlane(), slm);
            if (cmc.IsInPlace())
                pm.local_plane.distance = 0;
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
