#pragma once

/// The RadialMenusMode enum defines possible modes for radial menus.
///
/// \ingroup Enums
enum class RadialMenusMode {
    kDisabled,      ///< Don't use radial menus.
    kLeftForBoth,   ///< Use left menu configuration for both hands.
    kRightForBoth,  ///< Use right menu configuration for both hands.
    kIndependent,   ///< Use each menu configuration for its hand.
};

