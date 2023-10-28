#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "Commands/Command.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(CommandList);
DECL_SHARED_PTR(CommandManager);
DECL_SHARED_PTR(SessionState);

/// The CommandManager is in charge of determining how to process commands and
/// also undoing and redoing them.
///
/// It maintains a registry of CommandFunc functions to invoke for each type of
/// Command (keyed by type name) to do or undo a Command of that type.
/// signature for such a function is:
///
///      void SomeFunc(Command &command, Command::Op operation)
///
/// (where operation is either Command::Op::kDo or Command::Op::kUndo).
///
/// \ingroup Managers
class CommandManager {
  public:
    /// Alias for command functions.
    using CommandFunc = std::function<void(Command &, Command::Op)>;

    // ------------------------------------------------------------------------
    // Registration and initialization.
    // ------------------------------------------------------------------------

    CommandManager();

    /// Registers a function to invoke to execute the named type of command.
    void RegisterFunction(const Str &type_name, const CommandFunc &func);

    /// This is used when reading Commands from a file. It executes all
    /// Commands in the given CommandList after resetting all state.
    void ProcessCommandList(const CommandListPtr &command_list);

    /// Resets the CommandList to its default (empty) state.
    void ResetCommandList();

    /// Convenience that returns the SessionState from the AppInfo in the
    /// CommandList.
    const SessionStatePtr & GetSessionState() const;

    // ------------------------------------------------------------------------
    // Command storage and execution.
    // ------------------------------------------------------------------------

    /// Returns the CommandList instance used to manage commands.
    const CommandListPtr & GetCommandList() const { return command_list_; }

    /// Adds and executes the given command.
    void AddAndDo(const CommandPtr &command);

    /// Returns true if there is a command that can be undone.
    bool CanUndo() const;

    /// Returns true if there is a command that can be redone.
    bool CanRedo() const;

    /// Undoes the last executed command. Asserts if there isn't one.
    void Undo();

    /// Same as Undo(), but also purges the undone command as if it never
    /// happened. This may be called only for the last command.
    void UndoAndPurge();

    /// Returns the most recently executed command. Asserts if there is none.
    const CommandPtr & GetLastCommand() const;

    /// Redoes the last undone command. Asserts if there isn't one.
    void Redo();

    /// Simulates the effect of applying the given Command. This can be used
    /// during interaction before the Command is finalized, at which point it
    /// can be passed to AddAndDo().
    void SimulateDo(const CommandPtr &command);

    /// Returns a flag that is true when the CommandManager is validating
    /// commands while loading a session and false otherwise.
    bool IsValidating() const { return is_validating_; }

  private:
    /// Registry mapping Command type name to CommandFunc to execute.
    std::unordered_map<Str, CommandFunc> command_registry_;

    /// CommandList that manages the Command instances.
    CommandListPtr command_list_;

    /// Flag that is true while validating commands in a CommandList while
    /// loading a session file.
    bool is_validating_ = false;

    /// Performs a do or undo operation for the given Command.
    void Execute_(Command &command, Command::Op operation);

    /// Executes the given Command for validation when loading a session.
    void ExecuteForValidation_(const CommandPtr &command);
};
