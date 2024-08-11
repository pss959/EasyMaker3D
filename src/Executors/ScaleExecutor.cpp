//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ScaleExecutor.h"

#include <ion/math/vectorutils.h>

#include "Commands/ScaleCommand.h"
#include "Managers/SelectionManager.h"
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

ScaleExecutor::ExecData_ & ScaleExecutor::GetExecData_(Command &command) {
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

        // Scaling may also have to modify the position to compensate for the
        // motion due to scaling about a different point.
        switch (sc.GetMode()) {
          case ScaleCommand::Mode::kAsymmetric:
            AdjustTranslation_(sc.GetRatios(), pm);
            break;
          case ScaleCommand::Mode::kCenterSymmetric:
            // No adjustment necessary.
            break;
          case ScaleCommand::Mode::kBaseSymmetric:
            AdjustBaseTranslation_(pm);
            break;
        }
    }
}

void ScaleExecutor::AdjustTranslation_(const Vector3f &ratios,
                                       ExecData_::PerModel &pm) {
    // Compute the change in size in all 3 dimensions.
    auto &model = *pm.path_to_model.GetModel();
    const Bounds bounds = model.GetBounds();
    Vector3f change(0, 0, 0);
    for (int dim = 0; dim < 3; ++dim) {
        // A negative ratio indicates that the maximum side is fixed.
        ASSERT(ratios[dim] != 0);
        const bool is_max_fixed = ratios[dim] < 0;

        // Use the difference between the old and new scale values to determine
        // how much to move.
        change[dim] = (pm.old_scale[dim] - pm.new_scale[dim]) *
            (is_max_fixed ? bounds.GetMaxPoint()[dim] :
             bounds.GetMinPoint()[dim]);
    }

    pm.new_translation = pm.old_translation + (model.GetRotation() * change);
    model.SetTranslation(pm.new_translation);
}

void ScaleExecutor::AdjustBaseTranslation_(ExecData_::PerModel &pm) {
    auto &model = *pm.path_to_model.GetModel();

    // To compute the new height, transform the Y axis into Model coordinates
    // and determine how its length changes proportionally with the old and new
    // scales. Apply the same ratio to the Y translation.
    const Vector3f model_y    = -model.GetRotation() * Vector3f::AxisY();
    const float    old_length = ion::math::Length(pm.old_scale * model_y);
    const float    new_length = ion::math::Length(pm.new_scale * model_y);
    const float    ratio      = new_length / old_length;
    pm.new_translation = pm.old_translation;
    pm.new_translation[1] *= ratio;
    model.SetTranslation(pm.new_translation);
}
