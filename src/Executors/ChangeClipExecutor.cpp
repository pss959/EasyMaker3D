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
        cm.SetPlane(operation == Command::Op::kDo ?
                    pm.new_plane : pm.old_plane);
        AdjustTranslation_(cm);
    }

    // Reselect if undo or if command is finished being done.
    // XXXX Still needed?
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

            // Convert plane from stage to object coordinates.
            const Matrix4f som = SG::CoordConv(path).GetRootToObjectMatrix();
            pm.new_plane = TransformPlane(ccc.GetPlane(), som);
            ClippedModel &cm = GetTypedModel<ClippedModel>(pm.path_to_model);
            pm.old_plane = cm.GetPlane();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}

void ChangeClipExecutor::AdjustTranslation_(ClippedModel &cm) {
    // The original mesh and the clipped mesh are both centered on the origin
    // in their object coordinates (like all meshes). However, they are
    // different points in local coordinates due to the mesh size change. The
    // ClippedModel stores the offset for this purpose. The translations of the
    // two models differ by this offset in local coordinates.
    const Vector3f offset = cm.GetModelMatrix() * cm.GetMeshOffset();
    cm.SetTranslation(cm.GetOriginalModel()->GetTranslation() - offset);
}
