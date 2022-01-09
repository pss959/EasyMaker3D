#include "Managers/CommandManager.h"

#include "Util/Assert.h"

CommandManager::CommandManager() :
    command_list_(Parser::Registry::CreateObject<CommandList>()) {
    // Set up a default CommandList.
    command_list_->Reset();
}

void CommandManager::RegisterFunction(const std::string &type_name,
                                      const CommandFunc &func) {
    ASSERT(! Util::MapContains(command_registry_, type_name));
    command_registry_[type_name] = func;
}

void CommandManager::SetCommandList(const CommandListPtr &command_list) {
    command_list_ = command_list;
}

void CommandManager::ResetCommandList() {
    command_list_->Reset();
}

SessionState & CommandManager::GetSessionState() const {
    ASSERT(command_list_);
    ASSERT(command_list_->GetAppInfo());
    ASSERT(command_list_->GetAppInfo()->GetSessionState());
    return *command_list_->GetAppInfo()->GetSessionState();
}

void CommandManager::AddAndDo(const CommandPtr &command) {
    // Add the Command.
    ASSERT(command);
    command_list_->AddCommand(command);

    // Mark the command as being finalized.
    command->SetIsFinalized();

    // Invoke the pre-Do function.
    if (pre_do_func_)
        pre_do_func_(*command);

    // Execute it.
    Execute_(*command, Command::Op::kDo);
}

void CommandManager::Undo() {
    ASSERT(command_list_->CanUndo());
    const CommandPtr &command = command_list_->ProcessUndo();
    Execute_(*command, Command::Op::kUndo);
    if (post_undo_func_)
        post_undo_func_(*command);
}

void CommandManager::UndoAndPurge() {
    Undo();
    command_list_->RemoveLastCommand();
}

void CommandManager::Redo() {
    ASSERT(command_list_->CanRedo());
    const CommandPtr &command = command_list_->ProcessRedo();
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
    ASSERTM(it != command_registry_.end(),
            "No function for command: " + command.GetTypeName());
    it->second(command, operation);
}
