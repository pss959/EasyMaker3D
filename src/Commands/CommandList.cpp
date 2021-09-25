#include "Commands/CommandList.h"

#include "Assert.h"

void CommandList::Reset() {
    GetCommands().clear();
    current_index_  = 0;
    index_at_clear_ = 0;
}

void CommandList::AddCommand(const CommandPtr &command) {
    // If there are commands after the current index, add them as orphans to
    // the current command if necessary.
    auto &commands = GetCommands();
    if (current_index_ < commands.size()) {
        if (ShouldOrphanCommands_())
            OrphanCommands_(command);

        // Either way, remove them from the main list.
        ClearOrphanedCommands_();
    }
    commands.push_back(command);
    current_index_ = commands.size();
}

const CommandPtr & CommandList::GetCommand(size_t index) const {
    auto &commands = GetCommands();
    ASSERT(index < commands.size());
    return commands[index];
}

const CommandPtr & CommandList::GetCommandToUndo() const {
    ASSERT(CanUndo());
    return GetCommands()[GetIndexOfNextCommandToUndo_()];
}

const CommandPtr & CommandList::GetCommandToRedo() const {
    ASSERT(CanRedo());
    return GetCommands()[GetIndexOfNextCommandToRedo_()];
}

const CommandPtr & CommandList::ProcessUndo() {
    ASSERT(CanUndo());
    current_index_ = GetIndexOfNextCommandToUndo_();
    return GetCommands()[current_index_];
}

const CommandPtr & CommandList::ProcessRedo() {
    ASSERT(CanRedo());
    size_t index = GetIndexOfNextCommandToRedo_();
    current_index_ = index + 1;
    return GetCommands()[index];
}

void CommandList::RemoveLastCommand() {
    GetCommands().pop_back();
}

int CommandList::GetIndexOfNextCommandToUndo_() const {
    // Skip over any commands that have no undo effect.
    auto &commands = GetCommands();
    int i = current_index_ - 1;
    while (i >= 0 && ! commands[i]->HasUndoEffect())
        --i;
    return i;
}

int CommandList::GetIndexOfNextCommandToRedo_() const {
    // Skip over any commands that have no redo effect.
    auto &commands = GetCommands();
    size_t i = current_index_;
    while (i < commands.size() && ! commands[i]->HasRedoEffect())
        ++i;
    return i < commands.size() ? i : -1;
}

bool CommandList::ShouldOrphanCommands_() const {
    // TODO: A more thorough test would check to see if any of the potentially
    // orphaned commands creates a Model that is copied to the clipboard.
    // Right now, just check for a command that returns true for
    // ShouldBeAddedAsOrphan().
    auto &commands = GetCommands();
    ASSERT(current_index_ < commands.size());
    for (size_t i = current_index_; i < commands.size(); ++i) {
        if (commands[i]->ShouldBeAddedAsOrphan())
            return true;
    }
    return false;
}

void CommandList::OrphanCommands_(const CommandPtr &command) {
    auto &commands = GetCommands();
    ASSERT(current_index_ < commands.size());

    // Add the orphaned commands to the new command.
    command->AddOrphanedCommands(std::vector<CommandPtr>(
                                     commands.begin() + current_index_,
                                     commands.end()));
}

void CommandList::ClearOrphanedCommands_() {
    auto &commands = GetCommands();
    ASSERT(current_index_ < commands.size());
    commands.erase(commands.begin() + current_index_, commands.end());
}
