#include "Executors/ChangeBevelExecutor.h"

#include "Commands/ChangeBevelCommand.h"
#include "Models/BeveledModel.h"

void ChangeBevelExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeBevelCommand &cbc = GetTypedCommand<ChangeBevelCommand>(command);

    if (operation == Command::Op::kDo) {
        const Bevel new_bevel = cbc.GetBevel();
        for (auto &pm: data.per_model) {
            BeveledModel &bev = GetTypedModel<BeveledModel>(pm.path_to_model);
            bev.SetBevel(new_bevel);
        }
    }
    else {  // Undo.
        for (auto &pm: data.per_model) {
            BeveledModel &bev = GetTypedModel<BeveledModel>(pm.path_to_model);
            bev.SetBevel(pm.old_bevel);
        }
    }

    // There is no need to reselect for a Panel-based Tool.
}

ChangeBevelExecutor::ExecData_ & ChangeBevelExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeBevelCommand &cbc = GetTypedCommand<ChangeBevelCommand>(command);

        const auto &model_names = cbc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model  = FindPathToModel(model_names[i]);
            BeveledModel &bev = GetTypedModel<BeveledModel>(pm.path_to_model);
            pm.old_bevel = bev.GetBevel();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
