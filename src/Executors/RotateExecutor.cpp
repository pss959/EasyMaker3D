#include "Executors/RotateExecutor.h"

#include <ion/math/transformutils.h>

#include "Commands/RotateCommand.h"
#include "CoordConv.h"
#include "Math/Linear.h"
#include "Util/Assert.h"

void RotateExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    if (operation == Command::Op::kDo) {
        RotateCommand &rc = GetTypedCommand<RotateCommand>(command);
        if (rc.IsInPlace())
            RotateInPlace_(rc, data);
        else
            RotateAroundPrimary_(rc, data);
    }
    else {  // Undo.
        for (auto &pm: data.per_model) {
            auto model = pm.path_to_model.GetModel();
            model->SetRotation(pm.old_rotation);
            model->SetTranslation(pm.old_translation);
        }
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

RotateExecutor::ExecData_ & RotateExecutor::GetExecData_(Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        RotateCommand &rc = GetTypedCommand<RotateCommand>(command);

        const auto &model_names = rc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model = FindPathToModel(model_names[i]);
            auto model = pm.path_to_model.GetModel();
            pm.old_rotation    = model->GetRotation();
            pm.old_translation = model->GetTranslation();
            pm.new_rotation    = pm.old_rotation;
            pm.new_translation = pm.old_translation;
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}

void RotateExecutor::RotateInPlace_(const RotateCommand &rc, ExecData_ &data) {
    for (auto &pm: data.per_model) {
        // Restore the Model's rotation to make everything work relative to the
        // start of the rotation.
        auto model = pm.path_to_model.GetModel();
        model->SetRotation(pm.old_rotation);

        // Convert the rotation into stage coordinates.
        const Matrix4f osm =
            CoordConv(pm.path_to_model).GetObjectToRootMatrix();
        const Rotationf rot = GetStageRotation_(rc, osm);

        // Update the rotation. The translation should not need to change.
        pm.new_rotation = ComposeRotations(pm.old_rotation, rot);
        model->SetRotation(pm.new_rotation);
    }
}

void RotateExecutor::RotateAroundPrimary_(const RotateCommand &rc,
                                          ExecData_ &data) {
    // Restore the primary Model's rotation and position to make everything
    // work relative to the start of the rotation.
    const auto &pm0 = data.per_model[0];
    auto primary = pm0.path_to_model.GetModel();
    primary->SetRotation(pm0.old_rotation);
    primary->SetTranslation(pm0.old_translation);

    // Convert the center and rotation into stage coordinates for the primary
    // Model.
    const Matrix4f osm0 = CoordConv(pm0.path_to_model).GetObjectToRootMatrix();
    const Point3f primary_center = osm0 * Point3f::Zero();
    const Rotationf rot = GetStageRotation_(rc, osm0);

    for (auto &pm: data.per_model) {
        auto model = pm.path_to_model.GetModel();

        // The primary Model stays put.
        if (model == primary) {
            pm.new_rotation = ComposeRotations(pm.old_rotation, rot);
            model->SetRotation(pm.new_rotation);
        }
        // All other Models rotate about the primary's center in stage
        // coordinates.
        else {
            // Undo any changes from previous rotations so that the local
            // center is always the same point during the rotation.
            model->SetRotation(pm.old_rotation);
            model->SetTranslation(pm.old_translation);

            // Compute the center of the Model in stage coordinates.
            const Matrix4f osm =
                CoordConv(pm.path_to_model).GetObjectToRootMatrix();
            const Point3f center = osm * Point3f::Zero();

            // Rotate the Model and move it so its center is at the rotated
            // center (relative to the primary's center).
            pm.new_rotation = ComposeRotations(pm.old_rotation, rot);
            model->SetRotation(pm.new_rotation);
            model->MoveCenterTo(
                primary_center + rot * (center - primary_center));
            pm.new_translation = model->GetTranslation();
        }
    }
}

Rotationf RotateExecutor::GetStageRotation_(const RotateCommand &rc,
                                            const Matrix4f &mat) {
    Rotationf rot = rc.GetRotation();

    // If not axis-aligned, transform by the matrix.
    if (! rc.IsAxisAligned()) {
        Anglef   angle;
        Vector3f axis;
        rot.GetAxisAndAngle(&axis, &angle);
        rot = Rotationf::FromAxisAndAngle(mat * axis, angle);
    }

    return rot;
}
