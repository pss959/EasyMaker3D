#pragma once

#include "Executors/Executor.h"
#include "Tests/SceneTestBase.h"

/// Base class for Executor tests; it provides some conveniences to simplify and
/// clarify tests.
///
/// \ingroup Tests
class ExecutorTestBase : public SceneTestBase {
  protected:
    /// Parses and returns a Command of the templated type using the given
    /// contents string.
    template <typename T> std::shared_ptr<T> ParseCommand(
        const Str &type_name, const Str &contents) {
        SetParseTypeName(type_name);
        return ParseTypedObject<T>(type_name + "{ " + contents + " }");
    }

    /// Sets up a test Executor::Context with a RootModel and SelectionManager
    /// and stores it in the given Executor. Returns the context. If \p
    /// init_targets is true, this also sets up the TargetManager in the
    /// context with target widgets.
    Executor::Context & InitContext(Executor &exec, bool init_targets = false);
};
