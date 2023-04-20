#include "Executors/ChangeMirrorExecutor.h"

#include <ion/math/transformutils.h>

#include "Commands/ChangeMirrorCommand.h"
#include "Managers/SelectionManager.h"
#include "Math/Linear.h"
#include "Models/MirroredModel.h"

void ChangeMirrorExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    if (operation == Command::Op::kDo) {
        ChangeMirrorCommand &cmc =
            GetTypedCommand<ChangeMirrorCommand>(command);

        for (auto &pm: data.per_model) {
            MirroredModel &mm = GetTypedModel<MirroredModel>(pm.path_to_model);

            // Convert the plane from stage coordinates into object coordinates.
            const auto cc = pm.path_to_model.GetCoordConv();
            const Plane &stage_plane = cmc.GetPlane();
            const Plane object_plane =
                TransformPlane(stage_plane, cc.GetRootToObjectMatrix());

            // Set the plane in the MirroredModel.
            mm.SetPlaneNormal(object_plane.normal);

            // Translate to the other side of the stage plane.
            const Point3f center =
                cc.LocalToRoot(Point3f(mm.GetOperandModel()->GetTranslation()));
            mm.SetTranslation(cc.RootToLocal(stage_plane.MirrorPoint(center)));
        }
    }
    else {
        for (auto &pm: data.per_model) {
            MirroredModel &mm = GetTypedModel<MirroredModel>(pm.path_to_model);
            mm.SetPlaneNormal(pm.old_object_plane_normal);
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
        ChangeMirrorCommand &cmc =
            GetTypedCommand<ChangeMirrorCommand>(command);

        const auto &model_names = cmc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());

        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm   = data->per_model[i];
            const SelPath        path = FindPathToModel(model_names[i]);
            const MirroredModel  &mm  = GetTypedModel<MirroredModel>(path);
            pm.path_to_model           = path;
            pm.old_object_plane_normal = mm.GetPlaneNormal();
            pm.old_translation         = mm.GetTranslation();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
