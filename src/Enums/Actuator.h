//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

/// The Actuator enum defines Interface components that can actuate clicking or
/// dragging operations.
///
/// \ingroup Enums
enum class Actuator {
    kMouse,
    kLeftPinch,
    kRightPinch,
    kLeftGrip,
    kRightGrip,
    kLeftTouch,
    kRightTouch,
    kNone,         // Must be last so that its index is not used.
};
