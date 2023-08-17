#pragma once

#include "Tests/SceneTestBase.h"

/// Base class for Command tests; it provides some conveniences to simplify and
/// clarify tests.
/// \ingroup Tests
class CommandTestBase : public SceneTestBase {
  protected:
    /// This can be used for testing the GetDescription() function for derived
    /// Command classes. It parses a Command of the type passed to
    /// TestBaseWithTypes::SetParseTypeName() with the given contents and
    /// verifies that GetDescription() starts with \p desc_start.
    void TestDesc(const Str &contents, const Str &desc_start);
};
