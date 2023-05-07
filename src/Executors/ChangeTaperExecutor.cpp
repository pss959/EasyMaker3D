#include "Executors/ChangeTaperExecutor.h"

#include "Commands/ChangeTaperCommand.h"
#include "Models/TaperedModel.h"

void ChangeTaperExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeTaperCommand &cbc = GetTypedCommand<ChangeTaperCommand>(command);

    for (auto &pm: data.per_model) {
        TaperedModel &bev = GetTypedModel<TaperedModel>(pm.path_to_model);
        bev.SetTaper(operation == Command::Op::kDo ?
                     cbc.GetTaper() : pm.old_taper);
    }

    // There is no need to reselect for a Panel-based Tool.
}

ChangeTaperExecutor::ExecData_ & ChangeTaperExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeTaperCommand &cbc = GetTypedCommand<ChangeTaperCommand>(command);

        const auto &model_names = cbc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model  = FindPathToModel(model_names[i]);
            TaperedModel &bev = GetTypedModel<TaperedModel>(pm.path_to_model);
            pm.old_taper = bev.GetTaper();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
