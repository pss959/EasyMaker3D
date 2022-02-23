#include "Executors/ScaleExecutor.h"

#include "Commands/ScaleCommand.h"
#include "CoordConv.h"
#include "Util/Assert.h"

void ScaleExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    if (operation == Command::Op::kDo) {
        ScaleCommand &sc = GetTypedCommand<ScaleCommand>(command);
        ScaleModels_(sc, data);
    }
    else {  // Undo.
        for (auto &pm: data.per_model) {
            auto model = pm.path_to_model.GetModel();
            model->SetScale(pm.old_scale);
            model->SetTranslation(pm.old_translation);
        }
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ScaleExecutor::ExecData_ & ScaleExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ScaleCommand &sc = GetTypedCommand<ScaleCommand>(command);

        const auto &model_names = sc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model = FindPathToModel(model_names[i]);
            auto model = pm.path_to_model.GetModel();
            pm.old_scale       = model->GetScale();
            pm.old_translation = model->GetTranslation();
            pm.new_scale       = pm.old_scale;
            pm.new_translation = pm.old_translation;
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}

void ScaleExecutor::ScaleModels_(const ScaleCommand &sc, ExecData_ &data) {
    // Get the proper scale ratios disregarding negative values for asymmetric
    // scales.
    Vector3f abs_ratios = sc.GetRatios();
    for (int i = 0; i < 3; ++i)
        abs_ratios[i] = std::abs(abs_ratios[i]);

    for (auto &pm: data.per_model) {
        // Update the scale.
        pm.new_scale = abs_ratios * pm.old_scale;
        pm.path_to_model.GetModel()->SetScale(pm.new_scale);

        // Asymmetric scaling may also have to modify the position to
        // compensate for the motion due to scaling about a different point.
        if (! sc.IsSymmetric())
            AdjustTranslation_(sc.GetRatios(), pm);
    }
}

void ScaleExecutor::AdjustTranslation_(const Vector3f &ratios,
                                       ExecData_::PerModel &pm) {
    const Vector3f change = ComputeSizeChange_(pm, ratios);
    auto model = pm.path_to_model.GetModel();
    pm.new_translation = pm.old_translation + (model->GetRotation() * change);
    model->SetTranslation(pm.new_translation);
}

Vector3f ScaleExecutor::ComputeSizeChange_(const ExecData_::PerModel &pm,
                                           const Vector3f &ratios) {
    auto model = pm.path_to_model.GetModel();
    const Bounds bounds = model->GetBounds();
    Vector3f change(0, 0, 0);
    for (int dim = 0; dim < 3; ++dim) {
        // A negative ratio indicates that the maximum side is fixed.
        ASSERT(ratios[dim] != 0);
        const bool is_max_fixed = ratios[dim] < 0;

        // Transform the side that should be fixed in place by the old and new
        // scale values. Use the difference between the two to determine how
        // much to move.
        change[dim] = (pm.old_scale[dim] - pm.new_scale[dim]) *
            (is_max_fixed ? bounds.GetMaxPoint()[dim] :
             bounds.GetMinPoint()[dim]);
    }
    return change;
}
