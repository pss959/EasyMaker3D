#pragma once

#include <memory>
#include <string>

#include "Enums/Hand.h"
#include "Math/Types.h"
#include "Parser/Object.h"
#include "RadialMenuInfo.h"
#include "UnitConversion.h"
#include "Util/FilePath.h"

namespace Parser { class Registry; }

class Settings;
typedef std::shared_ptr<Settings> SettingsPtr;

/// The Settings struct stores application settings. It is derived from
/// Parser::Object so the settings can be read from and written to files.
class Settings : public Parser::Object {
  public:
    /// Creates an instance with default values.
    static SettingsPtr CreateDefault();

    /// Returns the FilePath containing the current session file.
    FilePath GetLastSessionPath() const {
        return ToPath_(last_session_path_, false);
    }

    /// Returns the FilePath for the directory to use for session files.
    FilePath GetSessionDirectory() const {
        return ToPath_(session_directory_, true);
    }

    /// Returns the FilePath for the directory to use to import models.
    FilePath GetImportDirectory() const {
        return ToPath_(import_directory_, true);
    }

    /// Returns the FilePath for the directory to use to export models.
    FilePath GetExportDirectory() const {
        return ToPath_(export_directory_, true);
    }

    /// Returns the delay in seconds for tooltip display.
    float GetTooltipDelay() const { return tooltip_delay_; }

    /// Returns the UnitConversion used when importing models.
    const UnitConversion & GetImportUnitsConversion() const {
        return *import_units_.GetValue();
    }

    /// Returns the UnitConversion used when exporting models.
    const UnitConversion & GetExportUnitsConversion() const {
        return *export_units_.GetValue();
    }

    /// Returns the size of the build volume.
    const Vector3f & GetBuildVolumeSize() const { return build_volume_size_; }

    /// Returns the RadialMenuInfo for the left radial menu.
    const RadialMenuInfo & GetLeftRadialMenuInfo() const {
        return *left_radial_menu_.GetValue();
    }

    /// Returns the RadialMenuInfo for the right radial menu.
    const RadialMenuInfo & GetRightRadialMenuInfo() const {
        return *right_radial_menu_.GetValue();
    }

    /// Convenience to get the RadialMenuInfo for a given Hand.
    const RadialMenuInfo & GetRadialMenuInfo(Hand hand) {
        return hand == Hand::kLeft ?
            GetLeftRadialMenuInfo() : GetRightRadialMenuInfo();
    }

    /// Returns the mode (string) for radial menus. One of:
    ///  \li "Disabled"     = Don't use radial menus.
    ///  \li "LeftForBoth"  = Use left menu configuration for both hands.
    ///  \li "RightForBoth" = Use right menu configuration for both hands.
    ///  \li "Each"         = Use each menu configuration for its hand.
    const std::string & GetRadialMenusMode() const {
        return radial_menus_mode_;
    }

    void SetLastSessionPath(const FilePath &path);
    void SetSessionDirectory(const FilePath &path);
    void SetImportDirectory(const FilePath &path);
    void SetExportDirectory(const FilePath &path);
    void SetTooltipDelay(float seconds);
    void SetImportUnitsConversion(const UnitConversion &uc);
    void SetExportUnitsConversion(const UnitConversion &uc);
    void SetBuildVolumeSize(const Vector3f &size);
    void SetLeftRadialMenuInfo(const RadialMenuInfo &info);
    void SetRightRadialMenuInfo(const RadialMenuInfo &info);
    void SetRadialMenusMode(const std::string &mode);

    /// Copies values from another instance.
    void CopyFrom(const Settings &from) { CopyContentsFrom(from, true); }

  protected:
    Settings() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

  private:
    typedef Parser::TField<std::string>         PathField_;
    typedef Parser::ObjectField<RadialMenuInfo> RadialMenuField_;
    typedef Parser::ObjectField<UnitConversion> UnitsField_;
    typedef Parser::TField<Vector3f>            VolumeField_;

    /// \name Parsed Fields
    /// Note that there are no default values; they are set in CreationDone()
    /// if not supplied in the parsed contents.
    ///@{
    PathField_                  last_session_path_{"last_session_path_"};
    PathField_                  session_directory_{"session_directory"};
    PathField_                  import_directory_{"import_directory"};
    PathField_                  export_directory_{"export_directory"};
    Parser::TField<float>       tooltip_delay_{"tooltip_delay"};
    UnitsField_                 import_units_{"import_units"};
    UnitsField_                 export_units_{"export_units"};
    VolumeField_                build_volume_size_{"build_volume_size"};
    RadialMenuField_            left_radial_menu_{"left_radial_menu"};
    RadialMenuField_            right_radial_menu_{"right_radial_menu"};
    Parser::TField<std::string> radial_menus_mode_{"radial_menus_mode"};
    ///@}

    /// Converts a string from a PathField_ to a FilePath. If the path does not
    /// exist or if is_dir is true and the path is not a directory, this
    /// returns an empty path.
    static FilePath ToPath_(const std::string &path_string, bool is_dir);

    /// Sets this instance to all default values.
    void SetToDefaults_();

    friend class Parser::Registry;
};
