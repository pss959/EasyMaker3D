#pragma once

#include <string>

#include "Base/Memory.h"
#include "Enums/Hand.h"
#include "Enums/RadialMenusMode.h"
#include "Items/RadialMenuInfo.h"
#include "Items/UnitConversion.h"
#include "Math/Types.h"
#include "Parser/Object.h"
#include "Util/Enum.h"
#include "Util/FilePath.h"

DECL_SHARED_PTR(Settings);

namespace Parser { class Registry; }

/// The Settings struct stores application settings. It is derived from
/// Parser::Object so the settings can be read from and written to files.
///
/// \ingroup Items
class Settings : public Parser::Object {
  public:
    /// Creates an instance with default values.
    static SettingsPtr CreateDefault();

    /// Creates an instance with values copied from the given instance.
    static SettingsPtr CreateCopy(const Settings &from);

    /// \name Path Settings
    /// Each of these returns a FilePath of some sort. The FilePath is not
    /// guaranteed to be valid; the caller should check.
    ///@{

    /// Returns the FilePath containing the current session file.
    FilePath GetLastSessionPath() const {
        return FilePath(last_session_path_);
    }

    /// Returns the FilePath for the directory to use for session files.
    FilePath GetSessionDirectory() const {
        return FilePath(session_directory_);
    }

    /// Returns the FilePath for the directory to use to import models.
    FilePath GetImportDirectory() const {
        return FilePath(import_directory_);
    }

    /// Returns the FilePath for the directory to use to export models.
    FilePath GetExportDirectory() const {
        return FilePath(export_directory_);
    }

    ///@}

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

    /// Convenience to get the RadialMenuInfo for a given Hand. This takes the
    /// current RadialMenusMode into account. This should not be called if
    /// radial menus are disabled.
    const RadialMenuInfo & GetRadialMenuInfo(Hand hand) const;

    /// Returns the mode for radial menus.
    RadialMenusMode GetRadialMenusMode() const { return radial_menus_mode_; }

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
    void SetRadialMenusMode(RadialMenusMode mode);

    /// Copies values from another instance.
    void CopyFrom(const Settings &from) { CopyContentsFrom(from, true); }

  protected:
    Settings() {}

    virtual void AddFields() override;

  private:
    typedef Parser::TField<Str>                 PathField_;
    typedef Parser::ObjectField<RadialMenuInfo> RadialMenuField_;
    typedef Parser::ObjectField<UnitConversion> UnitsField_;
    typedef Parser::TField<Vector3f>            VolumeField_;

    /// \name Parsed Fields
    /// Note that there are no default values; they are set in CreationDone()
    /// if not supplied in the parsed contents.
    ///@{
    PathField_                         last_session_path_;
    PathField_                         session_directory_;
    PathField_                         import_directory_;
    PathField_                         export_directory_;
    Parser::TField<float>              tooltip_delay_;
    UnitsField_                        import_units_;
    UnitsField_                        export_units_;
    VolumeField_                       build_volume_size_;
    RadialMenuField_                   left_radial_menu_;
    RadialMenuField_                   right_radial_menu_;
    Parser::EnumField<RadialMenusMode> radial_menus_mode_;
    ///@}

    /// Sets this instance to all default values.
    void SetToDefaults_();

    friend class Parser::Registry;
};
