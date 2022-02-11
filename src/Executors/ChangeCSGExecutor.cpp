#include "Executors/ChangeCSGExecutor.h"

#include "Commands/ChangeCSGOperationCommand.h"
#include "Models/CSGModel.h"

void ChangeCSGExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeCSGOperationCommand &ccc =
        GetTypedCommand<ChangeCSGOperationCommand>(command);

    for (auto &pm: data.per_model) {
        CSGModelPtr csg =
            Util::CastToDerived<CSGModel>(pm.path_to_model.GetModel());
        ASSERT(csg);
        if (operation == Command::Op::kDo) {
            csg->SetOperation(ccc.GetNewOperation());
            csg->ChangeModelName(pm.new_name, false);
        }
        else {  // Undo.
            csg->SetOperation(pm.old_operation);
            csg->ChangeModelName(pm.old_name, false);
        }
    }
    GetContext().selection_manager->ReselectAll();
}

ChangeCSGExecutor::ExecData_ & ChangeCSGExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeCSGOperationCommand &ccc =
            GetTypedCommand<ChangeCSGOperationCommand>(command);

        const auto &model_names = ccc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model   = FindPathToModel(model_names[i]);
            CSGModelPtr csg =
                Util::CastToDerived<CSGModel>(pm.path_to_model.GetModel());
            ASSERT(csg);
            const std::string prefix = Util::EnumToWord(ccc.GetNewOperation());
            pm.old_operation = csg->GetOperation();
            pm.old_name      = csg->GetName();
            pm.new_name      = GetContext().name_manager->Create(prefix);
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
