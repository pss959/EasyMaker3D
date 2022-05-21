#include "Executors/LinearLayoutExecutor.h"

#include <ion/math/transformutils.h>

#include "App/CoordConv.h"
#include "Commands/LinearLayoutCommand.h"
#include "Managers/SelectionManager.h"

void LinearLayoutExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    if (operation == Command::Op::kDo) {
        auto &llc = GetTypedCommand<LinearLayoutCommand>(command);
        LayOutModels_(data, llc.GetOffset());
    }
    else {  // Undo.
        for (auto &pm: data.per_model)
            pm.path_to_model.GetModel()->SetTranslation(pm.old_translation);
    }

    GetContext().selection_manager->ReselectAll();
}

LinearLayoutExecutor::ExecData_ & LinearLayoutExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        auto &llc = GetTypedCommand<LinearLayoutCommand>(command);

        const auto &model_names = llc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model   = FindPathToModel(model_names[i]);
            pm.old_translation = pm.path_to_model.GetModel()->GetTranslation();
            pm.new_translation = pm.old_translation;
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}

void LinearLayoutExecutor::LayOutModels_(ExecData_ &data,
                                         const Vector3f &offset) {
    // Compute the position of the first Model in stage coordinates.
    const Matrix4f osm =
        CoordConv(data.per_model[0].path_to_model).GetObjectToRootMatrix();
    Point3f cur_position = osm * Point3f::Zero();

    // Move the center of each other Model center to the current position.
    for (auto &pm: data.per_model) {
        if (&pm != &data.per_model[0]) {
            auto &model = *pm.path_to_model.GetModel();
            model.MoveCenterTo(cur_position);
            pm.new_translation = model.GetTranslation();
        }
        cur_position += offset;
    }
}
