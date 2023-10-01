#include "Commands/ConvertTwistCommand.h"
#include "Executors/ConvertTwistExecutor.h"
#include "Tests/Executors/ConvertExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ConvertTwistExecutorTest : public ConvertExecutorTestBase {};

TEST_F(ConvertTwistExecutorTest, TypeName) {
    ConvertTwistExecutor exec;
    EXPECT_EQ("ConvertTwistCommand", exec.GetCommandTypeName());
}

TEST_F(ConvertTwistExecutorTest, ConvertTwistWithoutName) {
    // Set up a ConvertTwistCommand without result names. This cannot be created
    // by parsing because having no result names is a parse error.
    auto cmd = Command::CreateCommand<ConvertTwistCommand>();
    ConvertTwistExecutor exec;
    ConvertWithoutNames(*cmd, exec, "TwistedModel", "Twisted");
}

TEST_F(ConvertTwistExecutorTest, ConvertTwistWithName) {
    auto cmd = ParseCommand<ConvertTwistCommand>(
        "ConvertTwistCommand",
        R"(model_names: ["Box_0", "Box_1"],
           result_names: ["Twisted_0", "Twisted_1"])");
    ConvertTwistExecutor exec;
    ConvertWithNames(*cmd, exec, "TwistedModel", "Twisted");
}
