#include "Executors/ChangeRevSurfExecutor.h"

#include <ion/math/transformutils.h>

#include "Commands/ChangeRevSurfCommand.h"
#include "Models/RevSurfModel.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

/// Returns the translation offset (in local coordinates) for the given
/// RevSurfModel based on its center offset (in object coordinates).
static Vector3f GetCenterOffset_(const RevSurfModel &rsm) {
    return rsm.GetModelMatrix() * rsm.GetCenterOffset();
}

void ChangeRevSurfExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeRevSurfCommand &crc = GetTypedCommand<ChangeRevSurfCommand>(command);

    if (operation == Command::Op::kDo) {
        const Profile  new_profile     = crc.GetProfile();
        const Anglef  &new_sweep_angle = crc.GetSweepAngle();
        for (auto &pm: data.per_model) {
            RevSurfModel &rsm = GetTypedModel<RevSurfModel>(pm.path_to_model);
            rsm.SetProfile(new_profile);
            rsm.SetSweepAngle(new_sweep_angle);
            pm.new_translation = pm.base_translation + GetCenterOffset_(rsm);
            rsm.SetTranslation(pm.new_translation);
        }
    }
    else {  // Undo.
        for (auto &pm: data.per_model) {
            RevSurfModel &rsm = GetTypedModel<RevSurfModel>(pm.path_to_model);
            rsm.SetProfile(pm.old_profile);
            rsm.SetSweepAngle(pm.old_sweep_angle);
            rsm.SetTranslation(pm.old_translation);
        }
    }

    // There is no need to reselect for a Panel-based Tool.
}

ChangeRevSurfExecutor::ExecData_ & ChangeRevSurfExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeRevSurfCommand &crc =
            GetTypedCommand<ChangeRevSurfCommand>(command);

        const auto &model_names = crc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model  = FindPathToModel(model_names[i]);
            RevSurfModel &rsm = GetTypedModel<RevSurfModel>(pm.path_to_model);
            pm.old_profile     = rsm.GetProfile();
            pm.old_sweep_angle = rsm.GetSweepAngle();
            pm.old_translation = rsm.GetTranslation();
            pm.new_translation = pm.old_translation;

            // Compute the base translation, which has no offset.
            pm.base_translation = pm.old_translation - GetCenterOffset_(rsm);
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
