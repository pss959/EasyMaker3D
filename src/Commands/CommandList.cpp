#include "Commands/CommandList.h"

#include "Util/Assert.h"

void CommandList::AddFields() {
    Parser::Object::AddFields();
    AddField(app_info_);
    AddField(commands_);
    AddField(current_index_);
}

bool CommandList::IsValid(std::string &details) {
    if (! Parser::Object::IsValid(details))
        return false;
    if (! app_info_.GetValue()) {
        details = "Missing app_info field";
        return false;
    }
    if (current_index_.GetValue() > GetCommandCount()) {
        details = "Invalid current_index";
        return false;
    }
    return true;
}

void CommandList::Reset() {
    GetCommands().clear();
    current_index_  = 0;
    index_at_clear_ = 0;

    if (! app_info_.GetValue())
        app_info_ = AppInfo::CreateDefault();
}

void CommandList::AddCommand(const CommandPtr &command) {
    // If there are commands after the current index, add them as orphans to
    // the current command if necessary.
    if (current_index_ < GetCommandCount()) {
        if (ShouldOrphanCommands_())
            OrphanCommands_(command);

        // Either way, remove them from the main list.
        ClearOrphanedCommands();
    }
    commands_.Add(command);
    current_index_ = GetCommandCount();
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
    commands_.Remove(GetCommandCount() - 1);
}

void CommandList::ClearOrphanedCommands() {
    ASSERT(current_index_ < GetCommandCount());
    while (current_index_ < GetCommandCount())
        commands_.Remove(current_index_);
}

int CommandList::GetIndexOfNextCommandToUndo_() const {
    // Skip over any commands that have no undo effect.
    auto &commands = GetCommands();
    int i = static_cast<int>(current_index_) - 1;
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
    /// \todo A more thorough test would check to see if any of the potentially
    /// orphaned commands creates a Model that is copied to the clipboard.
    /// Right now, just check for a command that returns true for
    /// ShouldBeAddedAsOrphan().
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
