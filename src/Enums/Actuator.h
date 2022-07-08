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
