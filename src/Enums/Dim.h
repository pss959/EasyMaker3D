//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

/// The Dim enum represents one of the three principal dimensions or axes. Note
/// that depending on context this may refer to application coordinates (Y up)
/// or user coordinates (Z up).
///
/// \ingroup Enums
enum class Dim {
    kX,
    kY,
    kZ,
};
