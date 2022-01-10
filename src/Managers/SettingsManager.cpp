#include "Managers/SettingsManager.h"

#include "Defaults.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"

SettingsManager::SettingsManager() {
    // See if there is a file with settings.
    FilePath path = FilePath::Join(FilePath::GetSettingsDirPath(),
                                   FilePath("settings.txt"));

    if (path.Exists()) {
        // XXXX Read settings.
    }
    else {
        // Use default settings.
        const FilePath home = FilePath::GetHomeDirPath();
        settings_.session_directory = home;
        settings_.import_directory  = home;
        settings_.export_directory  = home;
        settings_.build_volume_size = Defaults::kBuildVolumeSize;
        settings_.radial_menus_mode = "Each";
        settings_.tooltip_delay     = Defaults::kTooltipDelay;
    }
}

void SettingsManager::SetSettings(const Settings &new_settings) {
    settings_ = new_settings;
    if (change_func_)
        change_func_(new_settings);
}
