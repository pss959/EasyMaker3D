#include "Executors/EdgeTargetExecutor.h"

#include "Commands/ChangeEdgeTargetCommand.h"
#include "Managers/TargetManager.h"

void EdgeTargetExecutor::Execute(Command &command, Command::Op operation) {
    TargetManager &target_manager = *GetContext().target_manager;

    // Make sure the target is visible so the change is noticeable.
    if (! target_manager.IsEdgeTargetVisible())
        target_manager.SetEdgeTargetVisible(true);

    ChangeEdgeTargetCommand &cptc =
        GetTypedCommand<ChangeEdgeTargetCommand>(command);

    if (operation == Command::Op::kDo)
        target_manager.SetEdgeTarget(*cptc.GetNewTarget());
    else
        target_manager.SetEdgeTarget(*cptc.GetOldTarget());
}
