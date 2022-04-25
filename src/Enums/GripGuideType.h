#pragma once

/// The GripGuideType enum is used to select a specific type of guide for a
/// GripGuide to display.
///
/// \ingroup Enums
enum class GripGuideType {
    /// Indicates that no hover highlighting is currently enabled.
    kNone,

    /// The typical guide. It is a stick perpendicular to the Controller
    /// hand with a sphere at the end. The sphere is the hover point.
    kBasic,

    /// Guide used for rotation: a pole aligned with the controller with a
    /// sphere and ring at the end. The sphere is the hover point.
    kRotation,
};
