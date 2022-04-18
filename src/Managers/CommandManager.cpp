#include "Managers/CommandManager.h"

#include "Commands/CommandList.h"
#include "Parser/Registry.h"
#include "SessionState.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

CommandManager::CommandManager() :
    command_list_(Parser::Registry::CreateObject<CommandList>()) {
    // Set up a default CommandList.
    ResetCommandList();
}

void CommandManager::RegisterFunction(const std::string &type_name,
                                      const CommandFunc &func) {
    ASSERT(! Util::MapContains(command_registry_, type_name));
    command_registry_[type_name] = func;
}

void CommandManager::ProcessCommandList(const CommandListPtr &command_list) {
    ResetCommandList();

    // Execute all commands in the list.
    const size_t count = command_list->GetCommandCount();
    for (size_t i = 0; i < count; ++i)
        ExecuteForValidation_(command_list->GetCommand(i));

    // Copy the AppInfo (and SessionState) from the new CommandList.
    command_list_->SetAppInfo(command_list->GetAppInfo());
}

void CommandManager::ResetCommandList() {
    command_list_->Reset();
}

const SessionStatePtr & CommandManager::GetSessionState() const {
    ASSERT(command_list_);
    ASSERT(command_list_->GetAppInfo());
    ASSERT(command_list_->GetAppInfo()->GetSessionState());
    return command_list_->GetAppInfo()->GetSessionState();
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
    KLOG('x', "Executing " << command->GetDescription());
    Execute_(*command, Command::Op::kDo);
}

bool CommandManager::CanUndo() const {
    return command_list_->CanUndo();
}

bool CommandManager::CanRedo() const {
    return command_list_->CanRedo();
}

void CommandManager::Undo() {
    ASSERT(command_list_->CanUndo());
    const CommandPtr &command = command_list_->ProcessUndo();
    KLOG('x', "Undoing " << command->GetDescription());
    Execute_(*command, Command::Op::kUndo);
    if (post_undo_func_)
        post_undo_func_(*command);
}

void CommandManager::UndoAndPurge() {
    Undo();
    command_list_->RemoveLastCommand();
}

const CommandPtr & CommandManager::GetLastCommand() const {
    return command_list_->GetCommandToUndo();
}

void CommandManager::Redo() {
    ASSERT(command_list_->CanRedo());
    const CommandPtr &command = command_list_->ProcessRedo();
    if (pre_do_func_)
        pre_do_func_(*command);
    KLOG('x', "Redoing " << command->GetDescription());
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

void CommandManager::ExecuteForValidation_(const CommandPtr &command) {
    KLOG('x', "Validating " << command->GetDescription());
    command->SetIsValidating(true);

    // Process orphaned commands first, if any.
    const auto &ocs = command->GetOrphanedCommands();
    if (! ocs.empty()) {
        // Execute all orphaned commands and then undo them.
        for (const auto &oc: ocs)
            ExecuteForValidation_(oc);
        for (auto it = ocs.rbegin(); it != ocs.rend(); ++it) {
            Command &oc = **it;
            if (oc.HasUndoEffect())
                Undo();
        }
        // Make sure the undone commands are not added as orphans again.
        command_list_->ClearOrphanedCommands();
    }

    // Add and execute the command.
    AddAndDo(command);
    command->SetIsValidating(false);
}
