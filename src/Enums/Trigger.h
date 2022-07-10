#pragma once

/// The Trigger enum defines different ways a click or drag interaction can be
/// triggered.
///
/// \ingroup Enums
enum class Trigger {
    kPointer,  ///< Pointer-based (mouse or pinch) click or drag.
    kGrip,     ///< Controller grip click or drag.
    kTouch,    ///< Controller touch click or drag.
};
