#include "Executors/ChangeTwistExecutor.h"

#include "Commands/ChangeTwistCommand.h"
#include "Managers/SelectionManager.h"
#include "Models/TwistedModel.h"

void ChangeTwistExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeTwistCommand &ctc = GetTypedCommand<ChangeTwistCommand>(command);

    for (auto &pm: data.per_model) {
        TwistedModel &tm = GetTypedModel<TwistedModel>(pm.path_to_model);
        tm.SetTwist(operation == Command::Op::kDo ?
                    ctc.GetTwist() : pm.old_twist);
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ChangeTwistExecutor::ExecData_ & ChangeTwistExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeTwistCommand &ctc = GetTypedCommand<ChangeTwistCommand>(command);

        const auto &model_names = ctc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());

        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            const SelPath path = FindPathToModel(model_names[i]);
            pm.path_to_model = path;
            TwistedModel &tm = GetTypedModel<TwistedModel>(pm.path_to_model);
            pm.old_twist = tm.GetTwist();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
