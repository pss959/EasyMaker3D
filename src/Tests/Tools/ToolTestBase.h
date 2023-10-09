#pragma once

#include <concepts>
#include <memory>

#include "Tools/Tool.h"
#include "Tests/SceneTestBase.h"

/// Base class for Tool tests; it provides some conveniences to simplify and
/// clarify tests.
///
/// \ingroup Tests
class ToolTestBase : public SceneTestBase {
  protected:
    Tool::ContextPtr context;

    /// Creates a Tool of the given type and sets it up with a Tool::Context,
    /// which it stores in #context. The name is assumed to be both the type
    /// name and the name of the Tool object itself.
    template <typename T>
    std::shared_ptr<T> InitTool(const Str &name) {
        static_assert(std::derived_from<T, Tool> == true);
        auto tool = ReadRealNode<T>(GetContentsString_(name), name);
        SetUpTool_(tool);
        return tool;
    }

    /// Registers a dummy Executor function with the CommandManager for the
    /// named Command that does nothing.
    void AddDummyCommandFunction(const Str &name);

  private:
    static Str GetContentsString_(const Str &name);

    /// Sets up a Tool with a Tool::Context.
    void SetUpTool_(const ToolPtr &tool);
};
