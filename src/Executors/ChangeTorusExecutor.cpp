#include "Executors/ChangeTorusExecutor.h"

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Commands/ChangeTorusCommand.h"
#include "Managers/SelectionManager.h"
#include "Math/Linear.h"
#include "Models/TorusModel.h"
#include "SG/CoordConv.h"
#include "Util/Tuning.h"

void ChangeTorusExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeTorusCommand &ccc = GetTypedCommand<ChangeTorusCommand>(command);

    for (auto &pm: data.per_model) {
        TorusModel &torus = GetTypedModel<TorusModel>(pm.path_to_model);

        if (operation == Command::Op::kDo) {
            // Convert the radius from stage coordinates into object coordinates
            // of the TorusModel. This is not perfect, but is reasonable.
            const float obj_radius =
                ion::math::Length(pm.path_to_model.GetCoordConv().RootToObject(
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
        if (pm.do_translate) {
            Vector3f trans = torus.GetTranslation();
            trans[1] = torus.GetScale()[1] * torus.GetInnerRadius();
            torus.SetTranslation(trans);
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

            // If the inner radius is changing and the torus is resting on the
            // Stage, translate it to match the inner radius.
            pm.do_translate = ccc.IsInnerRadius() && IsOnStage_(pm);
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}

bool ChangeTorusExecutor::IsOnStage_(const ExecData_::PerModel &pm) const {
    using ion::math::Normalized;

    const TorusModel &torus = GetTypedModel<TorusModel>(pm.path_to_model);
    const SG::CoordConv cc(pm.path_to_model);

    // The torus has to have its axis of symmetry very close to the Y axis.
    // and its bottom very close to Y=0.
    const Vector3f y_axis = Vector3f::AxisY();
    const Vector3f t_axis = Normalized(cc.ObjectToRoot(Vector3f::AxisY()));
    const Point3f  bottom(0, -torus.GetInnerRadius(), 0);
    return AreDirectionsClose(y_axis, t_axis, Anglef::FromDegrees(.01f)) &&
        std::abs(cc.ObjectToRoot(bottom)[1]) <= TK::kCloseToStageForScaling;
}
