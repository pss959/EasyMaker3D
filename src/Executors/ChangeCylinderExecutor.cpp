#include "Executors/ChangeCylinderExecutor.h"

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Commands/ChangeCylinderCommand.h"
#include "Models/CylinderModel.h"

void ChangeCylinderExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeCylinderCommand &ccc =
        GetTypedCommand<ChangeCylinderCommand>(command);

    for (auto &pm: data.per_model) {
        CylinderModel &cyl = GetTypedModel<CylinderModel>(pm.path_to_model);
        if (operation == Command::Op::kDo) {
            // Convert the radius from stage coordinates into object coordinates
            // of the CylinderModel. This is not perfect, but is reasonable.
            const float obj_radius =
                ion::math::Length(pm.path_to_model.GetStageToObjectMatrix() *
                                  Vector3f(ccc.GetNewRadius(), 0, 0));
            std::cerr << "XXXX SRAD = " << ccc.GetNewRadius()
                      << " ORAD = " << obj_radius << "\n";
            cyl.SetRadius(ccc.GetWhichRadius(), obj_radius);
        }
        else {  // Undo.
            cyl.SetRadius(ccc.GetWhichRadius(), pm.old_radius);
        }
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ChangeCylinderExecutor::ExecData_ & ChangeCylinderExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeCylinderCommand &ccc =
            GetTypedCommand<ChangeCylinderCommand>(command);

        const auto &model_names = ccc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model   = FindPathToModel(model_names[i]);
            CylinderModel &cyl = GetTypedModel<CylinderModel>(pm.path_to_model);
            pm.old_radius = cyl.GetRadius(ccc.GetWhichRadius());
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
