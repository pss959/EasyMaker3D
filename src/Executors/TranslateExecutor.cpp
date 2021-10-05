#include "Executors/TranslateExecutor.h"

#include "Commands/TranslateCommand.h"

void TranslateExecutor::Execute(Command &command, Command::Op operation) {
#if XXXX
    ExecData_ &data = GetExecData_(command);

    // Process the command.
    const Context &context = GetContext();

    // XXXX Do something...
#endif
}

TranslateExecutor::ExecData_ & TranslateExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ASSERT(dynamic_cast<TranslateCommand *>(&command));
        TranslateCommand &tc = static_cast<TranslateCommand &>(command);

        const auto &model_names = tc.GetModelNames();

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model   = FindPathToModel(model_names[i]);
            pm.old_translation = pm.path_to_model.GetModel()->GetTranslation();
            pm.new_translation = pm.old_translation;
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
