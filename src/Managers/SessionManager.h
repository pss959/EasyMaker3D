#pragma once

#include <memory>
#include <string>
#include <vector>

#include "AppInfo.h"
#include "Enums/FileFormat.h"
#include "Managers/CommandManager.h"
#include "Managers/SelectionManager.h"
#include "Math/UnitConversion.h"
#include "Models/RootModel.h"
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

    /// The constructor is passed the required managers and nodes.
    SessionManager(const CommandManagerPtr &command_manager,
                   const SelectionManagerPtr &selection_manager);

    /// Returns the AppInfo for the current session.
    const AppInfoPtr & GetAppInfo() const;

    /// Returns flags indicating how the current session has been modified.
    const Util::Flags<Modification> & GetModifications() const {
        return modifications_;
    }

    /// Creates a new session.
    void NewSession();

    /// Returns true if the current session can be saved to its existing file,
    /// meaning that something has changed.
    bool CanSaveSession() const;

    /// Saves the current session to the given path. Returns true if all went
    /// well.
    bool SaveSession(const Util::FilePath &path);

    /// Loads an existing session from the given path. Returns true if all went
    /// well.
    bool LoadSession(const Util::FilePath &path);

    /// Returns the path of the last loaded/saved session. This will be an
    /// empty path if there is none.
    const Util::FilePath & GetSessionPath() const;

    /// Returns true if there is a current Model that can be exported.
    bool CanExport() const;

    /// Exports the current Model to the given path in the given format using
    /// the given ConversionInfo.  Returns true if all went well.
    bool Export(const Util::FilePath &path, FileFormat format,
                const UnitConversion &conv);

  private:
    CommandManagerPtr   command_manager_;
    SelectionManagerPtr selection_manager_;

    Util::FilePath            session_path_;
    Util::Flags<Modification> modifications_;

    /// Saves the current session to the file specified by path. The given
    /// vector of strings, if any, is written as header comments. Returns true
    /// if all went well.
    bool SaveSessionWithComments_(const Util::FilePath &path,
                                  const std::vector<std::string> &comments);
};

typedef std::shared_ptr<SessionManager> SessionManagerPtr;
