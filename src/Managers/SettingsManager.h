#pragma once

#include <functional>

#include "Base/Memory.h"
#include "Util/FilePath.h"

DECL_SHARED_PTR(Settings);
DECL_SHARED_PTR(SettingsManager);

/// SettingsManager manages application settings. Settings are represented by
/// an instance of the Settings struct. The SettingsManager allows the settings
/// to be saved and loaded from a file.
///
/// \ingroup Managers
class SettingsManager {
  public:
    /// Typedef for function that is invoked when settings change.
    typedef std::function<void(const Settings &)> ChangeFunc;

    SettingsManager();

    /// Sets a function that is invoked when the current settings change.
    void SetChangeFunc(const ChangeFunc &func) { change_func_ = func; }

    /// Returns the current settings (read-only).
    const Settings & GetSettings() const { return *settings_; }

    /// Updates the settings to new values and notifies the change function, if
    /// any. Writes out the new settings.
    void SetSettings(const Settings &new_settings);

 private:
    ChangeFunc  change_func_;
    SettingsPtr settings_;

    SettingsPtr ReadSettings_(const FilePath &path);
    void        WriteSettings_();

    /// Returns the path to the settings file in the directory with the given
    /// path.
    static FilePath GetSettingsFilePath_(const FilePath &dir_path);
};
