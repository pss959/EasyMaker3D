#include "Executors/ChangeClipExecutor.h"

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Commands/ChangeClipCommand.h"
#include "Managers/SelectionManager.h"
#include "Math/Linear.h"
#include "Models/ClippedModel.h"

void ChangeClipExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    for (auto &pm: data.per_model) {
        ClippedModel &cm = GetTypedModel<ClippedModel>(pm.path_to_model);
        if (operation == Command::Op::kDo) {
            // Save the current mesh offset.
            const Vector3f old_offset = cm.GetMeshOffset();
            cm.AddPlane(pm.local_plane);
            AdjustTranslation_(old_offset, pm);
        }
        else {   // Undo.
            cm.RemoveLastPlane();
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

void ChangeClipExecutor::AdjustTranslation_(const Vector3f &old_mesh_offset,
                                            ExecData_::PerModel &pm) {
    auto &model = *pm.path_to_model.GetModel();

    // Make sure the mesh and its offset are up to date.
    model.GetMesh();

    // Since clipping can move the mesh if the ClippedModel itself is scaled or
    // rotated, compute the offset to negate that.
    pm.old_translation = model.GetTranslation();
    const Vector3f offset = old_mesh_offset - pm.old_translation;
    const Vector3f diff   = model.GetRotation() *
        (model.GetScale() * offset) - offset;
    pm.new_translation = pm.old_translation - diff;
    model.SetTranslation(pm.new_translation);
}
