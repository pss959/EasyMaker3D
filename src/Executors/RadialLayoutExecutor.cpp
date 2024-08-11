//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/RadialLayoutExecutor.h"

#include <cmath>

#include <ion/math/transformutils.h>

#include "Commands/RadialLayoutCommand.h"
#include "Managers/SelectionManager.h"
#include "Math/Curves.h"

void RadialLayoutExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    if (operation == Command::Op::kDo) {
        auto &rlc = GetTypedCommand<RadialLayoutCommand>(command);
        LayOutModels_(data, rlc);
    }
    else {  // Undo.
        for (auto &pm: data.per_model) {
            auto &model = *pm.path_to_model.GetModel();
            model.SetRotation(pm.old_rotation);
            model.SetTranslation(pm.old_translation);
        }
    }

    GetContext().selection_manager->ReselectAll();
}

RadialLayoutExecutor::ExecData_ & RadialLayoutExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        auto &rlc = GetTypedCommand<RadialLayoutCommand>(command);

        const auto &model_names = rlc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model   = FindPathToModel(model_names[i]);
            auto &model        = *pm.path_to_model.GetModel();
            pm.old_rotation    = model.GetRotation();
            pm.old_translation = model.GetTranslation();
            pm.new_rotation    = pm.old_rotation;
            pm.new_translation = pm.old_translation;
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}

void RadialLayoutExecutor::LayOutModels_(ExecData_ &data,
                                         const RadialLayoutCommand &rlc) {
    if (data.per_model.size() == 1U) {
        // Only one model to lay out; move it to the center and orient it along
        // the normal.
        MoveModel_(data.per_model[0], rlc.GetCenter(), rlc.GetNormal());
    }
    else {
        // Multiple Models: get the positions along a 3D arc.
        const auto positions = GetModelPositions_(data.per_model.size(), rlc);
        int index = 0;
        for (auto &pm: data.per_model)
            MoveModel_(pm, positions[index++], rlc.GetNormal());
    }
}

std::vector<Point3f> RadialLayoutExecutor::GetModelPositions_(
    size_t count, const RadialLayoutCommand &rlc) {

    // Get 2D circle or arc points in the correct direction.
    const CircleArc &arc = rlc.GetArc();
    const float arc_degrees = arc.arc_angle.Degrees();
    std::vector<Point2f> arc_points_2d;
    if (std::abs(arc_degrees) == 360) {  // Full circle.
        arc_points_2d = GetCirclePoints(count, rlc.GetRadius(), false, true);
        if (arc_degrees < 0) {  // Clockwise
            for (auto &p: arc_points_2d)
                p[1] = -p[1];
        }
    }
    else {                          // Partial arc.
        arc_points_2d = GetCircleArcPoints(count, rlc.GetRadius(), arc);
    }

    // Convert each point to 3D (+Y => -Z) and rotate it into the layout plane.
    const Rotationf rot =
        Rotationf::RotateInto(Vector3f::AxisY(), rlc.GetNormal());

    std::vector<Point3f> positions(count);
    for (size_t i = 0; i < count; ++i) {
        positions[i] = rlc.GetCenter() +
            rot * Point3f(arc_points_2d[i][0], 0, -arc_points_2d[i][1]);
    }
    return positions;
}

void RadialLayoutExecutor::MoveModel_(ExecData_::PerModel &pm,
                                      const Point3f &position,
                                      const Vector3f &normal) {
    auto &model = *pm.path_to_model.GetModel();
    model.MoveBottomCenterTo(position, normal);
    pm.new_rotation    = model.GetRotation();
    pm.new_translation = model.GetTranslation();
}
