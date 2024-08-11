//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

/// Registers all concrete types derived from Parser::Object with the
/// Parser::Registry().
///
/// \ingroup App
void RegisterTypes();

/// Clears the registry, primarily for unit tests.
///
/// \ingroup App
void UnregisterTypes();
