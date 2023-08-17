#include "Commands/ConvertBendCommand.h"
#include "Commands/ConvertBevelCommand.h"
#include "Commands/ConvertClipCommand.h"
#include "Commands/ConvertMirrorCommand.h"
#include "Commands/ConvertTaperCommand.h"
#include "Commands/ConvertTwistCommand.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// This tests the base ConvertCommand and all classes derived from it. The
/// tests are almost identical, so a function in the test class does most of
/// the work.
/// \ingroup Tests
class ConvertCommandTest : public CommandTestBase {
  protected:
    /// Tests the named command.
    void TestCommand(const Str &type_name, const Str &action);
};

void ConvertCommandTest::TestCommand(const Str &type_name, const Str &action) {
    SetParseTypeName(type_name);
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [ " BadName" ])", "Invalid model name");
    TestInvalid(R"(model_names: ["X"])", "Missing result names");
    TestInvalid(R"(model_names: ["X"], result_names: [" Y"])",
                "Invalid result model name");
    TestValid(R"(model_names: ["X"], result_names: ["Y"])");

    TestDesc(R"(model_names: ["X"], result_names: ["Y"])",
             "Applied " + action + R"( to Model "X")");
    TestDesc(R"(model_names: ["A", "B"], result_names: ["Y", "Z"])",
             "Applied " + action + " to 2 Models");
}

TEST_F(ConvertCommandTest, ResultNames) {
    // Need to use a derived concrete class here.
    auto cbc = Command::CreateCommand<ConvertBendCommand>();
    EXPECT_TRUE(cbc->GetResultNames().empty());
    cbc->SetResultNames(StrVec{ "Abc", "Def" });
    EXPECT_EQ(2U,    cbc->GetResultNames().size());
    EXPECT_EQ("Abc", cbc->GetResultNames()[0]);
    EXPECT_EQ("Def", cbc->GetResultNames()[1]);
}

TEST_F(ConvertCommandTest, ConvertBendCommand) {
    TestCommand("ConvertBendCommand", "bend");
}

TEST_F(ConvertCommandTest, ConvertBevelCommand) {
    TestCommand("ConvertBevelCommand", "bevel");
}

TEST_F(ConvertCommandTest, ConvertClipCommand) {
    TestCommand("ConvertClipCommand", "clip");
}

TEST_F(ConvertCommandTest, ConvertMirrorCommand) {
    TestCommand("ConvertMirrorCommand", "mirroring");
}

TEST_F(ConvertCommandTest, ConvertTaperCommand) {
    TestCommand("ConvertTaperCommand", "taper");
}

TEST_F(ConvertCommandTest, ConvertTwistCommand) {
    TestCommand("ConvertTwistCommand", "twist");
}
