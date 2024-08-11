//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

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
