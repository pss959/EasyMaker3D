#include "Executors/DeleteExecutor.h"

#include "Commands/DeleteCommand.h"
#include "Managers/SelectionManager.h"
#include "Util/Assert.h"

void DeleteExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    // Deselect first because selected Models will be removed.
    const auto &context = GetContext();
    context.selection_manager->DeselectAll();

    if (operation == Command::Op::kDo) {
        // Do not use the stored indices for deletion, since deleting children
        // may modify indices.
        for (auto &pm: data.per_model) {
            const auto &model  = pm.path_to_model.GetModel();
            const auto &parent = pm.path_to_model.GetParentModel();
            const int index = parent->GetChildModelIndex(model);
            pm.path_to_model.GetParentModel()->RemoveChildModel(index);
        }
    }
    else {  // Undo.
        Selection sel;
        for (auto &pm: data.per_model) {
            const auto &model  = pm.path_to_model.GetModel();
            const auto &parent = pm.path_to_model.GetParentModel();
            parent->InsertChildModel(pm.index, model);
            sel.Add(pm.path_to_model);
        }
        context.selection_manager->ChangeSelection(sel);
    }
}

DeleteExecutor::ExecData_ & DeleteExecutor::GetExecData_(Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        DeleteCommand &dc = GetTypedCommand<DeleteCommand>(command);

        const auto &model_names = dc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model = FindPathToModel(model_names[i]);
            const auto &parent = pm.path_to_model.GetParentModel();
            pm.index = parent->GetChildModelIndex(pm.path_to_model.GetModel());
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
