#include "Executors/ChangeNameExecutor.h"

#include "Commands/ChangeNameCommand.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Models/Model.h"

void ChangeNameExecutor::Execute(Command &command, Command::Op operation) {
    ChangeNameCommand &cnc = GetTypedCommand<ChangeNameCommand>(command);
    Str from_name, to_name;
    if (operation == Command::Op::kDo) {
        from_name = cnc.GetModelName();
        to_name   = cnc.GetNewName();
    }
    else {
        to_name   = cnc.GetModelName();
        from_name = cnc.GetNewName();
    }

    FindPathToModel(from_name).GetModel()->ChangeModelName(to_name, true);

    auto &context = GetContext();
    context.name_manager->Remove(from_name);
    context.name_manager->Add(to_name);
    context.selection_manager->ReselectAll();
}

