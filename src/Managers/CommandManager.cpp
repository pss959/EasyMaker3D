//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Managers/CommandManager.h"

#include <ranges>

#include "Commands/CommandList.h"
#include "Items/SessionState.h"
#include "Parser/Registry.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

CommandManager::CommandManager() :
    command_list_(Parser::Registry::CreateObject<CommandList>()) {
    // Set up a default CommandList.
    ResetCommandList();
}

void CommandManager::RegisterFunction(const Str &type_name,
                                      const CommandFunc &func) {
    ASSERT(! command_registry_.contains(type_name));
    command_registry_[type_name] = func;
}

void CommandManager::ProcessCommandList(const CommandListPtr &command_list) {
    ResetCommandList();

    is_validating_ = true;

    // Execute all commands in the list.
    const size_t count = command_list->GetCommandCount();
    for (size_t i = 0; i < count; ++i)
        ExecuteForValidation_(command_list->GetCommand(i));

    // Undo if necessary.
    while (command_list_->GetCurrentIndex() > command_list->GetCurrentIndex())
        Undo();

    // Copy the AppInfo (and SessionState) from the new CommandList.
    command_list_->SetAppInfo(command_list->GetAppInfo());

    is_validating_ = false;
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

    // Execute it. Note that the KLOG() may not contain complete information
    // because executing the Command may add to it.
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
        // Execute all orphaned commands (in reverse order) and then undo them.
        for (const auto &oc: ocs)
            ExecuteForValidation_(oc);
        for (const auto &oc: ocs | std::views::reverse) {
            if (oc->HasUndoEffect())
                Undo();
        }
        // Make sure the undone commands are not added as orphans again.
        command_list_->ClearOrphanedCommands();
    }

    // Add and execute the command.
    AddAndDo(command);
    command->SetIsValidating(false);
}
