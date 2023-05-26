#include "Executors/ChangeTwistExecutor.h"

#include "Commands/ChangeTwistCommand.h"
#include "Managers/SelectionManager.h"
#include "Models/TwistedModel.h"

void ChangeTwistExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeTwistCommand &ctc = GetTypedCommand<ChangeTwistCommand>(command);

    if (operation == Command::Op::kDo) {
        for (auto &pm: data.per_model) {
            TwistedModel &tm = GetTypedModel<TwistedModel>(pm.path_to_model);
            tm.SetTwist(ctc.GetTwist());
            pm.new_translation =
                pm.base_translation + tm.GetLocalCenterOffset();
            tm.SetTranslation(pm.new_translation);
        }
    }
    else {  // Undo.
        for (auto &pm: data.per_model) {
            TwistedModel &tm = GetTypedModel<TwistedModel>(pm.path_to_model);
            tm.SetTwist(pm.old_twist);
            tm.SetTranslation(pm.old_translation);
        }
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
            pm.old_twist       = tm.GetTwist();
            pm.old_translation = tm.GetTranslation();
            pm.new_translation = pm.old_translation;

            // Compute the base translation, which has no offset.
            pm.base_translation =
                pm.old_translation - tm.GetLocalCenterOffset();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
