#include "Managers/SettingsManager.h"

#include "Defaults.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"

SettingsManager::SettingsManager() {
    // See if there is a file with settings.
    FilePath path = FilePath::Join(FilePath::GetSettingsDirPath(),
                                   FilePath("settings.txt"));
    if (path.Exists()) {
        settings_ = ReadSettings_();
    }
    else {
        settings_ = Settings::CreateDefault();

        // Use default settings.
    }
}

void SettingsManager::SetSettings(const Settings &new_settings) {
    settings_->CopyFrom(new_settings);
    WriteSettings_();
    if (change_func_)
        change_func_(new_settings);
}

SettingsPtr SettingsManager::ReadSettings_() {
    // XXXX
    return SettingsPtr();
}

void SettingsManager::WriteSettings_() {
    // XXXX
}
