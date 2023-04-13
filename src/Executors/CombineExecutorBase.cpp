#include "Executors/CombineExecutorBase.h"

#include "Commands/CombineCommand.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Selection/Selection.h"
#include "Util/General.h"

void CombineExecutorBase::Execute(Command &command, Command::Op operation) {
    // Deselect everything so that changes to Models do not cause reselection.
    const Context &context = GetContext();
    context.selection_manager->DeselectAll();

    ExecData_ &data = GetExecData_(command);

    // Process the command.
    Selection sel;
    if (operation == Command::Op::kDo) {
        // Reparent all of the operand Models to the CombinedModel.
        for (const auto &path: data.paths_to_models) {
            const int index =
                context.root_model->GetChildModelIndex(path.GetModel());
            ASSERT(index >= 0);
            context.root_model->RemoveChildModel(index);
        }

        // Add the CombinedModel as a top-level model and select it.
        context.root_model->AddChildModel(data.combined_model);
        sel.Add(SelPath(context.root_model, data.combined_model));
        context.name_manager->Add(data.combined_model->GetName());
    }
    else {  // Undo.
        // Remove the CombinedModel as a top-level model.
        const int index =
            context.root_model->GetChildModelIndex(data.combined_model);
        ASSERT(index >= 0);
        context.root_model->RemoveChildModel(index);
        context.name_manager->Remove(data.combined_model->GetName());

        // Add the operand Models back as top-level Models and select them.
        for (const auto &path: data.paths_to_models) {
            context.root_model->AddChildModel(path.GetModel());
            sel.Add(path);
        }
    }
    context.selection_manager->ChangeSelection(sel);
}

CombineExecutorBase::ExecData_ & CombineExecutorBase::GetExecData_(
    Command &command) {

    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        CombineCommand &cc = GetTypedCommand<CombineCommand>(command);

        ExecData_ *data = new ExecData_;

        const auto &model_names = cc.GetModelNames();
        ASSERT(! model_names.empty());
        std::vector<ModelPtr> operand_models;
        data->paths_to_models.reserve(model_names.size());
        operand_models.reserve(model_names.size());
        for (const auto &name: model_names) {
            data->paths_to_models.push_back(FindPathToModel(name));
            operand_models.push_back(data->paths_to_models.back().GetModel());
        }

        const auto &result = CreateCombinedModel(command);
        result->SetOperandModels(operand_models);

        // Compensate for the centering offset.
        result->SetTranslation(result->GetCenterOffset());

        AddModelInteraction(*result);
        SetRandomModelColor(*result);

        data->combined_model = result;
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
