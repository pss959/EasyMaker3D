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
        if (operation == Command::Op::kDo) {
            mm.AddPlaneNormal(pm.object_plane_normal);
            mm.SetTranslation(pm.new_translation);
        }
        else {   // Undo.
            mm.RemoveLastPlaneNormal();
            mm.SetTranslation(pm.old_translation);
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
        ChangeMirrorCommand &cmc = GetTypedCommand<ChangeMirrorCommand>(command);

        const auto &model_names = cmc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());

        const Plane &stage_plane = cmc.GetPlane();

        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            const SelPath path = FindPathToModel(model_names[i]);
            pm.path_to_model = path;

            // Convert the plane normal into object coordinates for the
            // MirroredModel.
            pm.object_plane_normal =
                CoordConv(path).RootToObject(stage_plane.normal);

            // If operating in-place, there is no need to modify the
            // translation of the MirroredModel. If not, it needs to be
            // translated to the other side of the stage plane.
            pm.old_translation = pm.path_to_model.GetModel()->GetTranslation();
            if (cmc.IsInPlace()) {
                pm.new_translation = pm.old_translation;
            }
            else {
                const Point3f mp =
                    stage_plane.MirrorPoint(Point3f(pm.old_translation));
                pm.new_translation = Vector3f(mp);
            }
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
