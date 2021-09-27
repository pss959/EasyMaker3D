#pragma once

/// Registers all concrete types derived from Parser::Object with the
/// Parser::Registry().
void RegisterTypes();

/// Clears the registry, primarily for unit tests.
void UnregisterTypes();
