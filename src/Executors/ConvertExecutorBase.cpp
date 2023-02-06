#include "Executors/ConvertExecutorBase.h"

#include "Commands/ConvertCommand.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Selection/Selection.h"
#include "Util/General.h"

void ConvertExecutorBase::Execute(Command &command, Command::Op operation) {
    // Deselect everything so that changes to Models do not cause reselection.
    const Context &context = GetContext();
    context.selection_manager->DeselectAll();

    ExecData_ &data = GetExecData_(command);

    // Process the command.
    Selection sel;

    // Process all Models that are/were not already the correct type.
    for (auto &pm: data.per_model) {
        const ModelPtr &original = pm.path_to_model.GetModel();
        if (original != pm.converted_model) {
            ModelPtr old_model;
            ModelPtr new_model;
            if (operation == Command::Op::kDo) {
                old_model = original;
                new_model = pm.converted_model;
                context.name_manager->Add(new_model->GetName());
            }
            else {  // Undo
                old_model = pm.converted_model;
                new_model = original;
                context.name_manager->Remove(old_model->GetName());
            }
            const int index = context.root_model->GetChildModelIndex(old_model);
            context.root_model->ReplaceChildModel(index, new_model);
            sel.Add(SelPath(context.root_model, new_model));
        }
    }

    context.selection_manager->ChangeSelection(sel);
}

ConvertExecutorBase::ExecData_ & ConvertExecutorBase::GetExecData_(
    Command &command) {

    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        const Context &context = GetContext();
        ConvertCommand &cc = GetTypedCommand<ConvertCommand>(command);

        const auto &model_names = cc.GetModelNames();
        ASSERT(! model_names.empty());

        // Add a PerModel instance for each Model. This includes those Models
        // that are already the correct type.
        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model = FindPathToModel(model_names[i]);

            // Pass the ConvertedModel for the primary selection to the
            // conversion function in case it is needed. Note that this will be
            // null until after the first iteration is done.
            pm.original_model = pm.path_to_model.GetModel();
            pm.converted_model = ConvertModel(
                pm.original_model, data->per_model[0].converted_model);

            // If conversion occurred, set up the new model.
            if (pm.converted_model != pm.original_model) {
                // Temporarily add the new Model's name so that all names will
                // be unique.
                context.name_manager->Add(pm.converted_model->GetName());
                AddModelInteraction(*pm.converted_model);
                SetRandomModelColor(*pm.converted_model);
            }
        }

        // Now that all converted Models have been created and named, remove
        // all of their names.
        for (const auto &pm: data->per_model)
            if (pm.converted_model != pm.original_model)
                context.name_manager->Remove(pm.converted_model->GetName());

        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
