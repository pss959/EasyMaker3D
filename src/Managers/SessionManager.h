#pragma once

#include <functional>
#include <string>
#include <vector>

#include "Base/Memory.h"
#include "Enums/FileFormat.h"
#include "Util/Enum.h"
#include "Util/FilePath.h"
#include "Util/Flags.h"

DECL_SHARED_PTR(ActionManager);
DECL_SHARED_PTR(CommandManager);
DECL_SHARED_PTR(SelectionManager);
DECL_SHARED_PTR(SessionManager);
DECL_SHARED_PTR(SessionState);
class UnitConversion;

/// The SessionManager handles saving and loading sessions.
///
/// \ingroup Managers
class SessionManager {
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
    SessionManager(const ActionManagerPtr &action_manager,
                   const CommandManagerPtr &command_manager,
                   const SelectionManagerPtr &selection_manager,
                   const FilePath &previous_path);

    /// Returns flags indicating how the current session has been modified.
    Util::Flags<Modification> GetModifications() const;

    /// Returns true if a session was started. This means that either a session
    /// was loaded or modifications were made to a new session.
    bool SessionStarted() const;

    /// Creates a new session.
    void NewSession();

    /// Returns true if the current session can be saved to its existing file,
    /// meaning that something has changed.
    bool CanSaveSession() const;

    /// Saves the current session to the given path. Returns true if all went
    /// well.
    bool SaveSession(const FilePath &path);

    /// Loads an existing session from the given path. Returns false and sets
    /// error to an informative string if anything fails.
    bool LoadSession(const FilePath &path, std::string &error);

    /// Returns true if there is a current Model that can be exported.
    bool CanExport() const;

    /// Exports the current Model to the given path in the given format using
    /// the given ConversionInfo.  Returns true if all went well.
    bool Export(const FilePath &path, FileFormat format,
                const UnitConversion &conv);

    /// Returns the name of current session, which may be empty.
    const std::string & GetSessionName() const { return session_name_; }

    /// Returns a string representing the current session: the name of the
    /// session and characters representing the current modifications.
    std::string GetSessionString() const;

  private:
    ActionManagerPtr    action_manager_;
    CommandManagerPtr   command_manager_;
    SelectionManagerPtr selection_manager_;
    std::string         session_name_;

    /// This saves the original SessionState. The current state is compared to
    /// this to determine if a change was made, allowing the session to be
    /// saved.
    SessionStatePtr original_session_state_;

    /// Resets the current session.
    void ResetSession_();

    /// Saves the current session to the file specified by path. The given
    /// vector of strings, if any, is written as header comments. Returns true
    /// if all went well.
    bool SaveSessionWithComments_(const FilePath &path,
                                  const std::vector<std::string> &comments);

    /// Loads a session from a path. If error is not null, this stores an error
    /// message in at and returns false if anything went wrong. Otherwise, it
    /// just throws the exception.
    bool LoadSessionSafe_(const FilePath &path, std::string *error);

    /// Sets the current session path and updates anything else in the app that
    /// depends on it.
    void SetSessionPath_(const FilePath &path);

    /// Changes the original session state to the current session state.
    void SaveOriginalSessionState_();
};
