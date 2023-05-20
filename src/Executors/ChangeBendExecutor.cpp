#include "Executors/ChangeBendExecutor.h"

#include "Commands/ChangeBendCommand.h"
#include "Managers/SelectionManager.h"
#include "Models/BentModel.h"

void ChangeBendExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeBendCommand &ctc = GetTypedCommand<ChangeBendCommand>(command);

    for (auto &pm: data.per_model) {
        BentModel &tm = GetTypedModel<BentModel>(pm.path_to_model);
        tm.SetBend(operation == Command::Op::kDo ?
                    ctc.GetBend() : pm.old_bend);
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ChangeBendExecutor::ExecData_ & ChangeBendExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeBendCommand &ctc = GetTypedCommand<ChangeBendCommand>(command);

        const auto &model_names = ctc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());

        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            const SelPath path = FindPathToModel(model_names[i]);
            pm.path_to_model = path;
            BentModel &tm = GetTypedModel<BentModel>(pm.path_to_model);
            pm.old_bend = tm.GetBend();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
