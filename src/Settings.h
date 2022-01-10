#pragma once

#include <string>

#include "Enums/Hand.h"
#include "Math/Types.h"
#include "Math/UnitConversion.h"
#include "RadialMenuInfo.h"
#include "Util/FilePath.h"

/// The Settings struct stores application settings.
struct Settings {
    /// Path to the last session file used; empty if there is none.
    FilePath       last_session_path;

    /// Directory path used for saving sessions.
    FilePath       session_directory;

    /// Directory path used for importing models.
    FilePath       import_directory;

    /// Directory path used for exporting models.
    FilePath       export_directory;

    /// Unit conversion information used when importing models.
    UnitConversion import_unit_conversion;

    /// Unit conversion information used when exporting models.
    UnitConversion export_unit_conversion;

    /// Size of the build volume.
    Vector3f       build_volume_size;

    /// Radial menu preferences for the left hand controller.
    RadialMenuInfo left_radial_menu_info;

    /// Radial menu preferences for the right hand controller.
    RadialMenuInfo right_radial_menu_info;

    /// The mode (string) for radial menus. One of:
    ///  \li "Disabled"     = Don't use radial menus.
    ///  \li "LeftForBoth"  = Use left menu configuration for both hands.
    ///  \li "RightForBoth" = Use right menu configuration for both hands.
    ///  \li "Each"         = Use each menu configuration for its hand.
    std::string    radial_menus_mode;

    /// Delay in seconds for tooltip display.
    float          tooltip_delay;

    /// Convenience to get the RadialMenuInfo for a given Hand.
    RadialMenuInfo & GetRadialMenuInfo(Hand hand) {
        return hand == Hand::kLeft ?
            left_radial_menu_info : right_radial_menu_info;
    }
};
