//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

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

