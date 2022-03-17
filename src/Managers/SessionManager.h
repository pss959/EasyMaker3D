#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Enums/FileFormat.h"
#include "Managers/ActionManager.h"
#include "Managers/CommandManager.h"
#include "Managers/SelectionManager.h"
#include "Models/RootModel.h"
#include "UnitConversion.h"
#include "Util/Enum.h"
#include "Util/FilePath.h"
#include "Util/Flags.h"

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

    /// The constructor is passed the required managers.
    SessionManager(const ActionManagerPtr &action_manager,
                   const CommandManagerPtr &command_manager,
                   const SelectionManagerPtr &selection_manager);

    /// Returns flags indicating how the current session has been modified.
    Util::Flags<Modification> GetModifications() const;

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

    /// Returns the path of the last loaded/saved session. This will be an
    /// empty path if there is none.
    const FilePath & GetSessionPath() const;

    /// Returns true if there is a current Model that can be exported.
    bool CanExport() const;

    /// Exports the current Model to the given path in the given format using
    /// the given ConversionInfo.  Returns true if all went well.
    bool Export(const FilePath &path, FileFormat format,
                const UnitConversion &conv);

    /// Returns a string representing the current session: the name of the
    /// session and characters representing the current modifications.
    std::string GetSessionString() const;

  private:
    ActionManagerPtr    action_manager_;
    CommandManagerPtr   command_manager_;
    SelectionManagerPtr selection_manager_;
    FilePath            session_path_;
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

typedef std::shared_ptr<SessionManager> SessionManagerPtr;
