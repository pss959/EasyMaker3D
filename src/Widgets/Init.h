#pragma once

namespace Parser { class Parser; }

/// Registers all concrete Widget types with the given Parser instance so they
/// can be parsed.
void RegisterWidgetTypes(Parser::Parser &parser);
