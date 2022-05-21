#include "Executors/ChangeTorusExecutor.h"

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "App/CoordConv.h"
#include "Commands/ChangeTorusCommand.h"
#include "Managers/SelectionManager.h"
#include "Models/TorusModel.h"

void ChangeTorusExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeTorusCommand &ccc = GetTypedCommand<ChangeTorusCommand>(command);

    for (auto &pm: data.per_model) {
        TorusModel &torus = GetTypedModel<TorusModel>(pm.path_to_model);
        if (operation == Command::Op::kDo) {
            // Convert the radius from stage coordinates into object coordinates
            // of the TorusModel. This is not perfect, but is reasonable.
            const float obj_radius =
                ion::math::Length(CoordConv(pm.path_to_model).RootToObject(
                                      Vector3f(ccc.GetNewRadius(), 0, 0)));
            if (ccc.IsInnerRadius())
                torus.SetInnerRadius(obj_radius);
            else
                torus.SetOuterRadius(obj_radius);
        }
        else {  // Undo.
            if (ccc.IsInnerRadius())
                torus.SetInnerRadius(pm.old_radius);
            else
                torus.SetOuterRadius(pm.old_radius);
        }
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ChangeTorusExecutor::ExecData_ & ChangeTorusExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeTorusCommand &ccc = GetTypedCommand<ChangeTorusCommand>(command);

        const auto &model_names = ccc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model   = FindPathToModel(model_names[i]);
            TorusModel &torus = GetTypedModel<TorusModel>(pm.path_to_model);
            pm.old_radius = ccc.IsInnerRadius() ?
                torus.GetInnerRadius() : torus.GetOuterRadius();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
