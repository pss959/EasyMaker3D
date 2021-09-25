#include "Managers/CommandManager.h"

#include "Assert.h"

void CommandManager::AddAndDo(const CommandPtr &command) {
    // Add the Command.
    ASSERT(command);
    command_list_.AddCommand(command);

    // Mark the command as being finalized.
    command->SetIsFinalized();

    // Invoke the pre-Do function.
    if (pre_do_func_)
        pre_do_func_(*command);

    // Execute it.
    Execute_(*command, Command::Op::kDo);
}

void CommandManager::Undo() {
    ASSERT(command_list_.CanUndo());
    const CommandPtr &command = command_list_.ProcessUndo();
    Execute_(*command, Command::Op::kUndo);
    if (post_undo_func_)
        post_undo_func_(*command);
}

void CommandManager::UndoAndPurge() {
    Undo();
    command_list_.RemoveLastCommand();
}

void CommandManager::Redo() {
    ASSERT(command_list_.CanRedo());
    const CommandPtr &command = command_list_.ProcessRedo();
    if (pre_do_func_)
        pre_do_func_(*command);
    Execute_(*command, Command::Op::kDo);
}

void CommandManager::SimulateDo(const CommandPtr &command) {
    Execute_(*command, Command::Op::kDo);
}

void CommandManager::Execute_(Command &command, Command::Op operation) {
    // Look up the command by its type name.
    auto it = command_registry_.find(command.GetTypeName());
    ASSERT(it != command_registry_.end());
    it->second(command, operation);
}
