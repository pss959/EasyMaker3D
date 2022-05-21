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
