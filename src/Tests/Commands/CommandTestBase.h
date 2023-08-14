#pragma once

#include "Tests/SceneTestBase.h"

/// Base class for Command tests; it provides some conveniences to simplify and
/// clarify tests.
/// \ingroup Tests
class CommandTestBase : public SceneTestBase {
  protected:
    /// This can be used for testing the GetDescription() function for derived
    /// Command classes. It parses the Command in the given string and verifies
    /// that GetDescription() starts with \p desc_start.
    void TestDesc(const std::string &str, const std::string &desc_start);
};
