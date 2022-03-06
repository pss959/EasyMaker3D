#include "Executors/ChangeNameExecutor.h"

#include "Commands/ChangeNameCommand.h"
#include "Models/Model.h"

void ChangeNameExecutor::Execute(Command &command, Command::Op operation) {
    ChangeNameCommand &cnc = GetTypedCommand<ChangeNameCommand>(command);
    std::string from_name, to_name;
    if (operation == Command::Op::kDo) {
        from_name = cnc.GetModelName();
        to_name   = cnc.GetNewName();
    }
    else {
        to_name   = cnc.GetModelName();
        from_name = cnc.GetNewName();
    }

    FindPathToModel(from_name).GetModel()->ChangeModelName(to_name, true);
    GetContext().selection_manager->ReselectAll();
}

