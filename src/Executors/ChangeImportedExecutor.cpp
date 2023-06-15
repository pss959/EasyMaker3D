#include "Executors/ChangeImportedExecutor.h"

#include "Commands/ChangeImportedModelCommand.h"
#include "Managers/SelectionManager.h"

void ChangeImportedExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    if (operation == Command::Op::kDo) {
        ChangeImportedModelCommand &cimc =
            GetTypedCommand<ChangeImportedModelCommand>(command);
        const bool was_loaded = data.imported_model->WasLoadedSuccessfully();
        data.imported_model->SetPath(cimc.GetNewPath());
        // Access the mesh so that errors can be detected.
        data.imported_model->GetMesh();
        // If this is the first time a real model was loaded successfully, set
        // the position of the ImportedModel.
        if (! was_loaded && data.imported_model->WasLoadedSuccessfully())
            InitModelPosition(*data.imported_model);
    }
    else {  // Undo.
        data.imported_model->SetPath(data.old_path);
    }

    // Do NOT reselect if there is a model error - that would cause the error
    // message dialog to go away.
    if (! data.imported_model->GetErrorMessage().empty())
        GetContext().selection_manager->ReselectAll();
}

ChangeImportedExecutor::ExecData_ & ChangeImportedExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeImportedModelCommand &cimc =
            GetTypedCommand<ChangeImportedModelCommand>(command);

        ExecData_ *data = new ExecData_;
        const auto path_to_model = FindPathToModel(cimc.GetModelName());
        ImportedModelPtr im =
            std::dynamic_pointer_cast<ImportedModel>(path_to_model.GetModel());
        ASSERT(im);
        data->imported_model = im;
        data->old_path       = im->GetPath();

        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
