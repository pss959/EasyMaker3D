#pragma once

#include <concepts>
#include <memory>

#include "Commands/Command.h"
#include "Tools/Tool.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// Base class for Tool tests; it provides some conveniences to simplify and
/// clarify tests.
///
/// \ingroup Tests
class ToolTestBase : public SceneTestBase {
  protected:
    Tool::ContextPtr context;

    virtual void TearDown() override;

    /// Creates a Tool of the given type and sets it up with a Tool::Context,
    /// which it stores in #context. The name is assumed to be both the type
    /// name and the name of the Tool object itself.
    template <typename T> std::shared_ptr<T> InitTool(const Str &name) {
        static_assert(std::derived_from<T, Tool> == true);
        auto tool = ReadRealNode<T>(GetContentsString_(), name);
        SetUpTool_(tool);
        return tool;
    }

    /// \name Command Testing
    /// Each of these can be used to verify that a Tool has added a specific
    /// type of Command to the CommandManager and optionally to test that it is
    /// executed.
    ///@{

    /// Registers a function with the CommandManager for the named Command that
    /// is invoked only for Command::Op::kDo.
    void AddCommandFunction(const Str &name,
                            const std::function<void(const Command &)> &func);

    /// Registers a dummy Executor function with the CommandManager for the
    /// named Command that does nothing.
    void AddDummyCommandFunction(const Str &name);

    /// Convenience that checks that the CommandManager has zero Commands.
    void CheckNoCommands();

    /// Convenience that checks that the CommandManager has exactly one Command
    /// and it is of the given derived type. Returns the derived Command so
    /// that further tests can be done by the caller.
    template <typename T> const T & CheckOneCommand() {
        return CheckLastCommand<T>(1);
    }

    /// Convenience that checks that the CommandManager has the given number of
    /// Commands and the last one is of the given derived type. Returns the
    /// derived Command so that further tests can be done by the caller.
    template <typename T> const T & CheckLastCommand(size_t count) {
        static_assert(std::derived_from<T, Command> == true);
        const auto *cmd = dynamic_cast<const T *>(CheckNCommands_(count));
        EXPECT_NOT_NULL(cmd);
        return *cmd;
    }

    ///@}

    /// \name Target Conveniences
    ///@{

    /// Convenience that set up the PointTarget in the TargetManager with the
    /// given position and direction and enables it.
    void SetPointTarget(const Point3f &pos, const Vector3f &dir);

    /// Convenience that set up the EdgeTarget in the TargetManager with the
    /// given length and enables it.
    void SetEdgeTargetLength(float length);

    ///@}

  private:
    static Str GetContentsString_();

    /// Sets up a Tool with a Tool::Context.
    void SetUpTool_(const ToolPtr &tool);

    /// Verifies that the CommandManager has \p count Commands and returns the
    /// last one.
    const Command * CheckNCommands_(size_t count);
};
