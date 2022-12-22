#include "Executors/ChangeImportedExecutor.h"

#include "Commands/ChangeImportedModelCommand.h"
#include "Managers/SelectionManager.h"
#include "Util/General.h"

void ChangeImportedExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    if (operation == Command::Op::kDo) {
        ChangeImportedModelCommand &cimc =
            GetTypedCommand<ChangeImportedModelCommand>(command);
        data.imported_model->SetPath(cimc.GetNewPath());
        // If this is the first time a real model is being imported, set the
        // position of the ImportedModel.
        if (data.old_path.empty())
            InitModelPosition(*data.imported_model);
    }
    else {  // Undo.
        data.imported_model->SetPath(data.old_path);
    }

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
            Util::CastToDerived<ImportedModel>(path_to_model.GetModel());
        ASSERT(im);
        data->imported_model = im;
        data->old_path       = im->GetPath();

        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
