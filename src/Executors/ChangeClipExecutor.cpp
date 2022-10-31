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
            // Save the unclipped Model bounds in object coordinates.
            const Bounds unclipped_bounds = cm.GetBounds();
            cm.AddPlane(pm.object_plane);
            AdjustTranslation_(unclipped_bounds, pm);
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

            // Convert plane from stage to object coordinates.
            const Matrix4f som = CoordConv(path).GetRootToObjectMatrix();
            pm.object_plane = TransformPlane(ccc.GetPlane(), som);
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}

void ChangeClipExecutor::AdjustTranslation_(const Bounds &unclipped_bounds,
                                            ExecData_::PerModel &pm) {
    auto &model = *pm.path_to_model.GetModel();

    // The original mesh and the clipped mesh are both centered on the origin
    // in object coordinates (like all meshes). However, they are different
    // points in local coordinates due to the mesh size change. Compute the
    // amount to translate to compensate for this change.
    //
    // The easiest way to do this is to make sure one of the unclipped points
    // of the original model ends up in the same place in local coordinates.
    // Take the corner of the object bounds that has the most negative distance
    // from the plane (also in object coordinates) and see how much it moves in
    // local coordinates, then translate by the opposite.

    const Bounds clipped_bounds = model.GetBounds();
    Point3f unclipped_corners[8];
    Point3f clipped_corners[8];
    unclipped_bounds.GetCorners(unclipped_corners);
    clipped_bounds.GetCorners(clipped_corners);

    int   stable_corner_index = -1;
    float most_neg_distance = 1;
    for (int i = 0; i < 8; ++i) {
        const float signed_distance =
            pm.object_plane.GetDistanceToPoint(unclipped_corners[i]);
        if (signed_distance < most_neg_distance) {
            stable_corner_index = i;
            most_neg_distance = signed_distance;
        }
    }
    ASSERT(stable_corner_index >= 0);

    // Convert the corner for both the original and clipped bounds to local
    // coordinates and get the difference.
    const Matrix4f &mm = model.GetModelMatrix();
    const Vector3f tr =
        (mm *   clipped_corners[stable_corner_index]) -
        (mm * unclipped_corners[stable_corner_index]);

    pm.old_translation = model.GetTranslation();
    pm.new_translation = pm.old_translation - tr;
    model.SetTranslation(pm.new_translation);
}
