#include "Executors/PointTargetExecutor.h"

#include "Commands/ChangePointTargetCommand.h"

void PointTargetExecutor::Execute(Command &command, Command::Op operation) {
    TargetManager &target_manager = *GetContext().target_manager;

    // Make sure the target is visible so the change is noticeable.
    if (! target_manager.IsPointTargetVisible())
        target_manager.SetPointTargetVisible(true);

    ChangePointTargetCommand &cptc =
        GetTypedCommand<ChangePointTargetCommand>(command);

    if (operation == Command::Op::kDo)
        target_manager.SetPointTarget(*cptc.GetNewTarget());
    else
        target_manager.SetPointTarget(*cptc.GetOldTarget());
}
