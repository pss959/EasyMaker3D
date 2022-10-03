#pragma once

#include <string>
#include <vector>

#include "App/Selection.h"
#include "Base/Memory.h"
#include "Parser/Object.h"

DECL_SHARED_PTR(Command);

/// Command is an abstract base class for all commands that can be executed,
/// undone, and redone. It is derived from Parser::Object so that Command
/// instances can be read from and written to files.
///
/// There are certain situations in which undone commands need to be retained,
/// even if other commands are performed afterwards. For example, consider this
/// sequence:
///
///  -# Create model M
///  -# Copy M
///  -# Undo (skips Copy, undoes creation of M)
///  -# Paste (pastes M, which is still in the clipboard)
///
/// If the first two commands are discarded, saving the session would have only
/// the PasteCommand; playing this session back would fail
/// miserably. Therefore, each Command has an \c orphaned_commands vector that
/// is usually empty. In the above example, the PasteCommand would have its \c
/// orphaned_commands vector set to contain the Create and Copy commands. The
/// \c orphaned_commands are saved with the session.  When playing back the
/// read-in session, the \c orphaned_commands are executed before the command
/// in which they appear. Note that any Command in the current list may have
/// entries in its \c orphaned_commands. If the same undo/redo pattern happens
/// more than once, the new orphaned commands are appended to the vector in the
/// command.
///
/// \ingroup Commands
class Command : public Parser::Object {
  public:
    /// Operations that can be applied to a Command.
    enum class Op {
        kDo,     ///< Execute or redo the Command.
        kUndo    ///< Undo the Command.
    };

    /// ExecData is a base struct that allows an Executor to cache data with
    /// the Command. Each Executor class can create its own version of ExecData
    /// to store in the exec_data field when the Command is first executed. The
    /// data will be available in subsequent calls to undo or redo the
    /// Command. Note that exec_data will be null the first time the Command is
    /// executed; an Executor can use that fact to tell whether it is the first
    /// time.
    struct ExecData {};

    virtual ~Command() {}

    /// Sets a flag indicating that the command contents are finalized. It
    /// allows Executors to know whether they are just simulating the Command
    /// during interaction or processing the final results. The CommandManager
    /// sets this to true when the Command is added.
    void SetIsFinalized() { is_finalized_ = true; }

    /// Returns the finalized flag, which is false until SetIsFinalized() is
    /// called.
    bool IsFinalized() const { return is_finalized_; }

    /// Sets a flag indicating that the command is being executed only to
    /// validate it while a session is being loaded. It is false otherwise.
    void SetIsValidating(bool is_validating) { is_validating_ = is_validating; }

    /// Returns the is-validating flag.
    bool IsValidating() const { return is_validating_; }

    /// Sets the Selection in place when the Command is first executed.
    void SetSelection(const Selection &sel) { selection_ = sel; }

    /// Returns the Selection in place when the Command was first executed.
    const Selection & GetSelection() const { return selection_; }

    /// Adds a vector of orphaned commands, which are undone commands that are
    /// no longer in the undo/redo list but should be executed before this
    /// command is executed.
    void AddOrphanedCommands(const std::vector<CommandPtr> &commands);

    /// Returns the orphaned commands stored in this Command.
    const std::vector<CommandPtr> & GetOrphanedCommands() const {
        return orphaned_commands_;
    }

    /// Returns a string describing the Command instance. This can be used, for
    /// example, for Undo/Redo tooltips.
    virtual std::string GetDescription() const = 0;

    /// Returns false if a command has no effect for an Undo operation, this
    /// returns false. The base class defines this to return true.
    virtual bool HasUndoEffect() const { return true; }

    /// Returns false if a command has no effect for a Redo operation. The base
    /// class defines this to return true.
    virtual bool HasRedoEffect() const { return true; }

    /// Returns true if this Command could potentially affect other commands
    /// when undone, and therefore should be added as an orphan. The base class
    /// defines this to return false.
    virtual bool ShouldBeAddedAsOrphan() const { return false; }

    /// Returns the ExecData instance stored in the command. This is null until
    /// SetExecData() is called.
    ExecData * GetExecData() const { return exec_data_.get(); }

    /// Sets the (derived) ExecData instance in the command. The Command takes
    /// ownership of the instance.
    void SetExecData(ExecData *data) { exec_data_.reset(data); }

  protected:
    /// Returns a description of the Model with the given name for use in
    /// GetDescription().
    static std::string GetModelDesc(const std::string &model_name);

    /// Returns a description of a vector of Models with the given names for
    /// use in GetDescription().
    static std::string GetModelsDesc(
        const std::vector<std::string> &model_names);

    /// "Fixes" a path if necessary to make it absolute. This is a hedge
    /// against someone editing a session file and using a relative path and is
    /// also needed for the snapimage application to allow for relative import.
    static std::string FixPath(const std::string &path_string);

  private:
    /// True when the Command is finalized after interaction.
    bool is_finalized_  = false;

    /// True when the Command is being executed for session validation.
    bool is_validating_ = false;

    /// Selection in place when the Command was first executed.
    Selection selection_;

    /// Orphaned commands for this Command; typically empty.
    std::vector<CommandPtr> orphaned_commands_;

    /// ExecData (typically a derived type) associated with the Command.
    std::unique_ptr<ExecData> exec_data_;
};
