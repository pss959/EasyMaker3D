#include "Executors/ChangeExtrudedExecutor.h"

#include <ion/math/transformutils.h>

#include "Commands/ChangeExtrudedCommand.h"
#include "Models/ExtrudedModel.h"

void ChangeExtrudedExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeExtrudedCommand &cec = GetTypedCommand<ChangeExtrudedCommand>(command);

    if (operation == Command::Op::kDo) {
        const Profile new_profile = cec.GetProfile();
        for (auto &pm: data.per_model) {
            ExtrudedModel &em = GetTypedModel<ExtrudedModel>(pm.path_to_model);
            em.SetProfile(new_profile);
        }
    }
    else {  // Undo.
        for (auto &pm: data.per_model) {
            ExtrudedModel &em = GetTypedModel<ExtrudedModel>(pm.path_to_model);
            em.SetProfile(pm.old_profile);
        }
    }

    // There is no need to reselect for a Panel-based Tool.
}

ChangeExtrudedExecutor::ExecData_ & ChangeExtrudedExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeExtrudedCommand &cec =
            GetTypedCommand<ChangeExtrudedCommand>(command);

        const auto &model_names = cec.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model  = FindPathToModel(model_names[i]);
            ExtrudedModel &em = GetTypedModel<ExtrudedModel>(pm.path_to_model);
            pm.old_profile    = em.GetProfile();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
