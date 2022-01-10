#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Commands/CommandList.h"
#include "SessionState.h"

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
    /// Typedef for command functions.
    typedef std::function<void(Command &, Command::Op)> CommandFunc;

    /// Typedef for pre-do and post-undo auxiliary functions.
    typedef std::function<void(Command &)> AuxFunc;

    // ------------------------------------------------------------------------
    // Registration and initialization.
    // ------------------------------------------------------------------------

    CommandManager();

    /// Registers a function to invoke to execute the named type of command.
    void RegisterFunction(const std::string &type_name,
                          const CommandFunc &func);

    /// This is used when reading Commands from a file. It executes all
    /// Commands in the given CommandList adter resetting all state.
    void ProcessCommandList(const CommandListPtr &command_list);

    /// Resets the CommandList to its default (empty) state.
    void ResetCommandList();

    /// Convenience that returns the SessionState from the AppInfo in the
    /// CommandList.
    SessionState & GetSessionState() const;

    /// Sets an auxiliary function to invoke just before doing or redoing a
    /// Command. It is null by default. The function is passed the Command
    /// being done or redone.
    void SetPreDoFunc(const AuxFunc &func) { pre_do_func_ = func; }

    /// Sets an auxiliary function to invoke just after undoing a Command. It
    /// is null by default. The function is passed the Command being undone.
    void SetPostUndoFunc(const AuxFunc &func) { post_undo_func_ = func; }

    // ------------------------------------------------------------------------
    // Command storage and execution.
    // ------------------------------------------------------------------------

    /// Returns the CommandList instance used to manage commands.
    CommandList & GetCommandList() { return *command_list_; }

    /// Adds and executes the given command.
    void AddAndDo(const CommandPtr &command);

    /// Returns true if there is a command that can be undone.
    bool CanUndo() const { return command_list_->CanUndo(); }

    /// Returns true if there is a command that can be redone.
    bool CanRedo() const { return command_list_->CanRedo(); }

    /// Undoes the last executed command. Asserts if there isn't one.
    void Undo();

    /// Same as Undo(), but also purges the undone command as if it never
    /// happened. This may be called only for the last command.
    void UndoAndPurge();

    /// Redoes the last undone command. Asserts if there isn't one.
    void Redo();

    /// Simulates the effect of applying the given Command. This can be used
    /// during interaction before the Command is finalized, at which point it
    /// can be passed to AddAndDo().
    void SimulateDo(const CommandPtr &command);

  private:
    /// Registry mapping Command type name to CommandFunc to execute.
    std::unordered_map<std::string, CommandFunc> command_registry_;

    /// Pre-Do function to invoke.
    AuxFunc pre_do_func_;

    /// Post-Undo function to invoke.
    AuxFunc post_undo_func_;

    /// CommandList that manages the Command instances.
    CommandListPtr command_list_;

    /// Performs a do or undo operation for the given Command.
    void Execute_(Command &command, Command::Op operation);

    /// Executes the given Command for validation when loading a session.
    void ExecuteForValidation_(const CommandPtr &command);
};

typedef std::shared_ptr<CommandManager> CommandManagerPtr;
