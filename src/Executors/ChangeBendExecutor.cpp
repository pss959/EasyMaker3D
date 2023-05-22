#include "Executors/ChangeBendExecutor.h"

#include "Commands/ChangeBendCommand.h"
#include "Managers/SelectionManager.h"
#include "Models/BentModel.h"

void ChangeBendExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeBendCommand &cbc = GetTypedCommand<ChangeBendCommand>(command);

    if (operation == Command::Op::kDo) {
        for (auto &pm: data.per_model) {
            BentModel &bm = GetTypedModel<BentModel>(pm.path_to_model);
            bm.SetBend(cbc.GetBend());
            pm.new_translation = pm.base_translation +
                ComputeLocalOffset(bm, bm.GetObjectCenterOffset());
            bm.SetTranslation(pm.new_translation);
        }
    }
    else {  // Undo.
        for (auto &pm: data.per_model) {
            BentModel &bm = GetTypedModel<BentModel>(pm.path_to_model);
            bm.SetBend(pm.old_bend);
            bm.SetTranslation(pm.old_translation);
        }
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ChangeBendExecutor::ExecData_ & ChangeBendExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeBendCommand &cbc = GetTypedCommand<ChangeBendCommand>(command);

        const auto &model_names = cbc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());

        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            const SelPath path = FindPathToModel(model_names[i]);
            pm.path_to_model = path;
            BentModel &bm = GetTypedModel<BentModel>(pm.path_to_model);
            pm.old_bend = bm.GetBend();
            pm.old_translation = bm.GetTranslation();
            pm.new_translation = pm.old_translation;

            // Compute the base translation, which has no offset.
            pm.base_translation = pm.old_translation -
                ComputeLocalOffset(bm, bm.GetObjectCenterOffset());
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
