#include "Items/Settings.h"

#include <vector>

#include "Base/Tuning.h"
#include "Parser/Registry.h"
#include "Util/Enum.h"

void Settings::AddFields() {
    AddField(last_session_path_.Init("last_session_path"));
    AddField(session_directory_.Init("session_directory"));
    AddField(import_directory_.Init("import_directory"));
    AddField(export_directory_.Init("export_directory"));
    AddField(tooltip_delay_.Init("tooltip_delay"));
    AddField(import_units_.Init("import_units"));
    AddField(export_units_.Init("export_units"));
    AddField(build_volume_size_.Init("build_volume_size"));
    AddField(left_radial_menu_.Init("left_radial_menu"));
    AddField(right_radial_menu_.Init("right_radial_menu"));
    AddField(radial_menus_mode_.Init("radial_menus_mode"));

    Parser::Object::AddFields();
}

SettingsPtr Settings::CreateDefault() {
    SettingsPtr settings = Parser::Registry::CreateObject<Settings>();
    settings->SetToDefaults_();
    return settings;
}

void Settings::SetLastSessionPath(const FilePath &path) {
    last_session_path_ = path.ToString();
}

void Settings::SetSessionDirectory(const FilePath &path) {
    session_directory_ = path.ToString();
}

void Settings::SetImportDirectory(const FilePath &path) {
    import_directory_ = path.ToString();
}

void Settings::SetExportDirectory(const FilePath &path) {
    export_directory_ = path.ToString();
}

void Settings::SetTooltipDelay(float seconds) {
    tooltip_delay_ = seconds;
}

void Settings::SetImportUnitsConversion(const UnitConversion &uc) {
    ASSERT(import_units_.GetValue());
    import_units_.GetValue()->CopyFrom(uc);
}

void Settings::SetExportUnitsConversion(const UnitConversion &uc) {
    ASSERT(export_units_.GetValue());
    export_units_.GetValue()->CopyFrom(uc);
}

void Settings::SetBuildVolumeSize(const Vector3f &size) {
    build_volume_size_ = size;
}

void Settings::SetLeftRadialMenuInfo(const RadialMenuInfo &info) {
    ASSERT(left_radial_menu_.GetValue());
    left_radial_menu_.GetValue()->CopyFrom(info);
}

void Settings::SetRightRadialMenuInfo(const RadialMenuInfo &info) {
    ASSERT(right_radial_menu_.GetValue());
    right_radial_menu_.GetValue()->CopyFrom(info);
}

void Settings::SetRadialMenusMode(RadialMenusMode mode) {
    radial_menus_mode_ = mode;
}

FilePath Settings::ToPath_(const std::string &path_string, bool is_dir) {
    FilePath path(path_string);
    if (! path.Exists() || (is_dir && ! path.IsDirectory()))
        path = FilePath();
    return path;
}

void Settings::SetToDefaults_() {
    const std::string home = FilePath::GetHomeDirPath().ToString();

    // last_session_path_ stays empty.
    session_directory_ = home;
    import_directory_  = home;
    export_directory_  = home;
    tooltip_delay_     = TK::kTooltipDelay;
    import_units_      = UnitConversion::CreateWithUnits(
        UnitConversion::Units::kMillimeters,
        UnitConversion::Units::kCentimeters);
    export_units_      = UnitConversion::CreateWithUnits(
        UnitConversion::Units::kCentimeters,
        UnitConversion::Units::kMillimeters);
    build_volume_size_ = TK::kBuildVolumeSize;
    left_radial_menu_  = Parser::Registry::CreateObject<RadialMenuInfo>();
    right_radial_menu_ = Parser::Registry::CreateObject<RadialMenuInfo>();
    radial_menus_mode_ = RadialMenusMode::kIndependent;
}
