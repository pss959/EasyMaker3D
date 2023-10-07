#pragma once

#include <functional>

#include "Agents/SettingsAgent.h"
#include "Base/Memory.h"
#include "Util/FilePath.h"

DECL_SHARED_PTR(Settings);
DECL_SHARED_PTR(SettingsManager);

/// SettingsManager manages application settings. Settings are represented by
/// an instance of the Settings class. The SettingsManager allows the settings
/// to be saved and loaded from a file.
///
/// \ingroup Managers
class SettingsManager : public SettingsAgent {
  public:
    /// Typedef for function that is invoked when settings change.
    typedef std::function<void(const Settings &)> ChangeFunc;

    /// The constructor installs default settings.
    SettingsManager();

    /// Sets the path to the settings file to load to initialize settings. If
    /// the path is to an existing file, this reads the settings from it.  If
    /// \p save_on_set is true, future calls to SetSettings() will save the new
    /// settings to the same path. This returns false if the path is not an
    /// existing file or if settings could not be read from it; the reason for
    /// the failure is stored in the \p error string.
    bool SetPath(const FilePath &path, bool save_on_set, Str &error);

    /// Sets a function that is invoked when the current settings change.
    void SetChangeFunc(const ChangeFunc &func) { change_func_ = func; }

    // SettingsAgent functions.
    virtual const Settings & GetSettings() const override { return *settings_; }
    virtual void SetSettings(const Settings &new_settings) override;

 private:
    FilePath    path_;
    bool        save_on_set_ = false;
    ChangeFunc  change_func_;
    SettingsPtr settings_;

    bool LoadSettings_(const FilePath &path, Str &error);
    void SaveSettings_(const FilePath &path);
};
