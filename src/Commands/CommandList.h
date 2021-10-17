#pragma once

#include <vector>

#include "Commands/Command.h"
#include "Parser/Object.h"

/// CommandList maintains a list of Commands that have been executed, allowing
/// them to be undone and redone. It is used by the CommandManager.
///
/// CommandList is derived from Parser::Object so the commands can be read from
/// and written to files.
///
/// \ingroup Commands
class CommandList : public Parser::Object {
  public:
    //! Resets when a new session starts.
    void Reset();

    /// Adds the given command.
    void AddCommand(const CommandPtr &command);

    /// Returns the total number of commands, including those that were undone.
    size_t GetCommandCount() const { return GetCommands().size(); }

    /// Returns the indexed command. Asserts if the index is bad.
    const CommandPtr & GetCommand(size_t index) const;

    /// Returns true if there is a command that can be undone.
    bool CanUndo() const { return GetIndexOfNextCommandToUndo_() >= 0; }

    /// Returns true if there is a command that can be redone.
    bool CanRedo() const { return GetIndexOfNextCommandToRedo_() >= 0; }

    /// Returns the next command to undo. This asserts if CanUndo() is false.
    const CommandPtr & GetCommandToUndo() const;

    /// Returns the next command to redo. This asserts if CanRedo() is false.
    const CommandPtr & GetCommandToRedo() const;

    /// Updates the commands for an undo operation, returning the undone
    /// command. Asserts if CanUndo() is false.
    const CommandPtr & ProcessUndo();

    /// Updates the commands for a redo operation. Asserts if CanRedo() is
    /// false.
    const CommandPtr & ProcessRedo();

    /// Removes the last command.
    void RemoveLastCommand();

    /// Marks the list as having had no changes, meaning that there is nothing
    /// to save.
    void ClearChanges() { index_at_clear_ = current_index_; }

    /// Returns true if any commands have been added since the last call to
    /// ClearChanges().
    bool WasAnyCommandAdded() const {
        return index_at_clear_ < GetCommandCount();
    }

    /// Returns true if any changes have been made since the last call to
    // ClearChanges().
    bool AreAnyChanges() const {
        return current_index_ > 0 && index_at_clear_ != current_index_;
    }

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<Command> commands_{"commands"};
    Parser::TField<unsigned int>     current_index_{"current_index", 0};
    ///@}

    /// This is used to determine what changes were made since the last call to
    /// ClearChanges(). It saves the value of current_index when ClearChanges()
    /// was called.
    size_t index_at_clear_ = 0;

    /// Accesses the Command vector from the Parser::ObjectListField.
    const std::vector<CommandPtr> & GetCommands() const {
        return commands_.GetValue();
    }

    /// Accesses the Command vector from the Parser::ObjectListField.
    std::vector<CommandPtr> & GetCommands() { return commands_.GetValue(); }

    /// Returns the index of the next command to undo, skipping any that have
    /// no undo effect. Returns -1 if there is nothing to undo.
    int GetIndexOfNextCommandToUndo_() const;

    /// Returns the index of the next command to redo, skipping any that have
    /// no redo effect. Returns -1 if there is nothing to redo.
    int GetIndexOfNextCommandToRedo_() const;

    /// Returns true if the Command instances after the current index should be
    /// added as orphans to the current Command.
    bool ShouldOrphanCommands_() const;

    /// Adds all Commands after the current one as orphans to the given Command
    /// and removes them from the main list.
    void OrphanCommands_(const CommandPtr &command);

    /// Removes all Commands after the current one from the main list.
    void ClearOrphanedCommands_();
};
