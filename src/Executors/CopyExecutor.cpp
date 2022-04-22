#include "Executors/CopyExecutor.h"

#include "Managers/ClipboardManager.h"
#include "Managers/SelectionManager.h"
#include "Util/Assert.h"

void CopyExecutor::Execute(Command &command, Command::Op operation) {
    // The CopyCommand cannot be undone or redone.
    ASSERT(operation == Command::Op::kDo);

    const auto &context = GetContext();
    context.clipboard_manager->StoreCopies(
        context.selection_manager->GetSelection().GetModels());
}
