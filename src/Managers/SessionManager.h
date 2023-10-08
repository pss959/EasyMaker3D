#pragma once

#include <string>
#include <vector>

#include "Agents/SessionAgent.h"
#include "Base/Memory.h"
#include "Util/Enum.h"
#include "Util/Flags.h"

DECL_SHARED_PTR(ActionAgent);
DECL_SHARED_PTR(CommandManager);
DECL_SHARED_PTR(SelectionManager);
DECL_SHARED_PTR(SessionManager);
DECL_SHARED_PTR(SessionState);
class UnitConversion;

/// The SessionManager handles saving and loading sessions.
///
/// \ingroup Managers
class SessionManager : public SessionAgent {
  public:
    /// Enum indicating how the current session has been modified.
    enum class Modification : uint32_t {
        /// A real change was made that affects something in the scene.
        kScene        = (1 << 0),

        /// Something in the session state has changed.
        kSessionState = (1 << 1),

        /// Commands were added (but may have been undone so the scene is back
        /// to its original condition).
        kCommands     = (1 << 2),
    };

    /// The constructor is passed the required managers and the file path for
    /// the previous session, if any.
    SessionManager(const ActionAgentPtr &action_agent,
                   const CommandManagerPtr &command_manager,
                   const SelectionManagerPtr &selection_manager,
                   const FilePath &previous_path);

    /// Changes the path used to display the previous session name.
    void ChangePreviousPath(const FilePath &previous_path);

    /// Returns flags indicating how the current session has been modified.
    Util::Flags<Modification> GetModifications() const;

    /// Returns a string representing the current session: the name of the
    /// session and characters representing the current modifications.
    Str GetSessionString() const;

    /// Saves the current session to the file specified by path. The given
    /// vector of strings, if any, is written as header comments. Returns true
    /// if all went well.
    bool SaveSessionWithComments(const FilePath &path, const StrVec &comments);

    // SessionAgent interface implementation.
    virtual void NewSession() override;
    virtual bool CanSaveSession() const override;
    virtual bool SaveSession(const FilePath &path) override;
    virtual bool LoadSession(const FilePath &path, Str &error) override;
    virtual Str GetModelNameForExport() const override;
    virtual bool Export(const FilePath &path, FileFormat format,
                        const UnitConversion &conv) override;
    virtual const Str & GetPreviousSessionName() const override;
    virtual const Str & GetCurrentSessionName() const override;

  private:
    ActionAgentPtr      action_agent_;
    CommandManagerPtr   command_manager_;
    SelectionManagerPtr selection_manager_;
    Str                 previous_session_name_;  ///< From previous run.
    Str                 current_session_name_;   ///< After load or save.

    /// This saves the original SessionState. The current state is compared to
    /// this to determine if a change was made, allowing the session to be
    /// saved.
    SessionStatePtr original_session_state_;

    /// Resets the current session.
    void ResetSession_();

    /// Loads a session from a path. Stores a message in \p error and returns
    /// false if anything went wrong.
    bool LoadSessionSafe_(const FilePath &path, Str &error);

    /// Changes the original session state to the current session state.
    void SaveOriginalSessionState_();

    /// Returns the name of a session from a FilePath.
    static Str GetSessionNameFromPath_(const FilePath &path);
};
