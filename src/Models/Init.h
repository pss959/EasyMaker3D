#pragma once

namespace Parser { class Parser; }

/// Registers all concrete Model types with the given Parser instance so they
/// can be parsed. This can be useful for testing.
void RegisterModelTypes(Parser::Parser &parser);
