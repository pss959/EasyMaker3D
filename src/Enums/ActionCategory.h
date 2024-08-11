//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

/// The ActionCategory enum defines categories that the Action enum values can
/// be partitioned into.
///
/// \ingroup Enums
enum class ActionCategory {
    kBasics,
    kCombination,
    kConversion,
    kCreation,
    kLayout,
    kModification,
    kNone,
    kPrecision,
    kRadialMenu,
    kSelection,
    kSession,
    kSpecialized,
    kTool,
    kViewing,
};
