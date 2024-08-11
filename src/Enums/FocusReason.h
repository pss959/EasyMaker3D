//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

/// The FocusReason enum represents a reason why the application wants to
/// change focus to a Pane.
///
/// \ingroup Enums
enum class FocusReason {
    kInitialFocus,   ///< Pane was indicated as initial focus for Panel.
    kActivation,     ///< Pane was activated.
    kMove,           ///< Focus was moved by user.
};
