#include "Executors/PasteExecutor.h"

#include "Commands/PasteCommand.h"
#include "Managers/ClipboardManager.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Models/RootModel.h"
#include "Util/Assert.h"

void PasteExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);
    const auto &context = GetContext();

    const auto &parent = data.path_to_parent.empty() ?
        context.root_model :
        std::dynamic_pointer_cast<ParentModel>(data.path_to_parent.GetModel());
    ASSERT(parent);


    // Tell the PasteCommand about the models so it can have an accurate
    // description string.
    PasteCommand &pc = GetTypedCommand<PasteCommand>(command);
    pc.SetModelsForDescription(data.models);

    Selection sel;
    if (operation == Command::Op::kDo) {
        for (auto &model: data.models) {
            parent->AddChildModel(model);

            // Select the child.
            SelPath sel_path = data.path_to_parent.empty() ?
                SelPath(context.root_model, model) :
                data.path_to_parent.GetPathToChild(model);
            sel.Add(sel_path);
        }
    }
    else {  // Undo.
        for (auto &model: data.models) {
            const int index = parent->GetChildModelIndex(model);
            parent->RemoveChildModel(index);
            RemoveNames_(*model);
        }
        // Select the parent if any.
        if (! data.path_to_parent.empty())
            sel.Add(data.path_to_parent);
    }
    GetContext().selection_manager->ChangeSelection(sel);
}

PasteExecutor::ExecData_ & PasteExecutor::GetExecData_(Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        PasteCommand &pc = GetTypedCommand<PasteCommand>(command);
        const auto &context = GetContext();
        ExecData_ *data = new ExecData_;

        // Clone the Models from the clipboard.
        data->models = context.clipboard_manager->CreateClones();

        // Change their colors to random ones, give them unique names, and
        // attach clicks (recursively).
        for (const auto &model: data->models)
            SetUpPastedModel_(*model);

        // If there is a named parent, store a path to it. Otherwise, leave it
        // empty.
        if (! pc.GetParentName().empty())
            data->path_to_parent = FindPathToModel(pc.GetParentName());

        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}

void PasteExecutor::SetUpPastedModel_(Model &model) {
    const auto &context = GetContext();

    // Change the color to a random one.
    SetRandomModelColor(model);

    // Assign a unique name. Pass true for is_user_edit so that the unique name
    // is always used.
    const Str new_name = context.name_manager->CreateClone(model.GetBaseName());
    model.ChangeModelName(new_name, true);
    context.name_manager->Add(new_name);

    // Set up click.
    AddModelInteraction(model);

    // Recurse if necessary.
    if (ParentModel *parent = dynamic_cast<ParentModel *>(&model)) {
        for (size_t i = 0; i < parent->GetChildModelCount(); ++i)
            SetUpPastedModel_(*parent->GetChildModel(i));
    }
}

void PasteExecutor::RemoveNames_(Model &model) {
    // Remove the name.
    GetContext().name_manager->Remove(model.GetName());

    // Recurse if necessary.
    if (ParentModel *parent = dynamic_cast<ParentModel *>(&model)) {
        for (size_t i = 0; i < parent->GetChildModelCount(); ++i)
            RemoveNames_(*parent->GetChildModel(i));
    }
}
