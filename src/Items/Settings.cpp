//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Items/Settings.h"

#include <vector>

#include "Parser/Registry.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/Tuning.h"

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

SettingsPtr Settings::CreateCopy(const Settings &from) {
    auto copy = CreateDefault();
    copy->CopyFrom(from);
    return copy;
}

const RadialMenuInfo & Settings::GetRadialMenuInfo(Hand hand) const {
    const auto mode = GetRadialMenusMode();
    ASSERT(mode != RadialMenusMode::kDisabled);
    const Hand hand_to_use =
        mode == RadialMenusMode::kLeftForBoth  ? Hand::kLeft  :
        mode == RadialMenusMode::kRightForBoth ? Hand::kRight : hand;
    return hand_to_use == Hand::kLeft ?
        GetLeftRadialMenuInfo() : GetRightRadialMenuInfo();
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

void Settings::CopyFrom(const Settings &from) {
    CopyContentsFrom(from, true);

    // Do not allow null objects for settings.
    SetToDefaults_();
}

void Settings::SetToDefaults_() {
    const Str home = FilePath::GetHomeDirPath().ToString();

    // Set unset values to default.
    //    Note that last_session_path_ can remain empty.
    if (! session_directory_.WasSet())
        session_directory_ = home;
    if (! import_directory_.WasSet())
        import_directory_  = home;
    if (! export_directory_.WasSet())
        export_directory_  = home;
    if (! tooltip_delay_.WasSet())
        tooltip_delay_     = TK::kTooltipDelay;
    if (! build_volume_size_.WasSet())
        build_volume_size_ = TK::kBuildVolumeSize;
    if (! radial_menus_mode_.WasSet())
        radial_menus_mode_ = RadialMenusMode::kIndependent;

    // Don't allow null values.
    if (! import_units_.GetValue())
        import_units_ = UnitConversion::CreateWithUnits(
            UnitConversion::Units::kMillimeters,
            UnitConversion::Units::kCentimeters);
    if (! export_units_.GetValue())
        export_units_ = UnitConversion::CreateWithUnits(
            UnitConversion::Units::kCentimeters,
            UnitConversion::Units::kMillimeters);
    if (! left_radial_menu_.GetValue())
        left_radial_menu_  = Parser::Registry::CreateObject<RadialMenuInfo>();
    if (! right_radial_menu_.GetValue())
        right_radial_menu_ = Parser::Registry::CreateObject<RadialMenuInfo>();
}
