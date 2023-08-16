#include "Commands/CopyCommand.h"
#include "Commands/DeleteCommand.h"
#include "Commands/PasteCommand.h"
#include "Models/BoxModel.h"
#include "Tests/Testing.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Util/Assert.h"

/// Base class for commands that are simple enough that they don't warrant
/// their own file.
/// \ingroup Tests
class OtherCommandTest : public CommandTestBase {};

TEST_F(OtherCommandTest, CopyCommand) {
    auto cc = Command::CreateCommand<CopyCommand>();
    EXPECT_FALSE(cc->HasUndoEffect());
    EXPECT_FALSE(cc->HasRedoEffect());
    EXPECT_TRUE(cc->ShouldBeAddedAsOrphan());

    SetParseTypeName("CopyCommand");
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [ " BadName" ])", "Invalid model name");
    TestValid(R"(model_names: ["Box"])");
    TestDesc(R"(model_names: ["Box"])",    R"(Copied Model "Box")");
    TestDesc(R"(model_names: ["A", "B"])",   "Copied 2 Models");
}

TEST_F(OtherCommandTest, DeleteCommand) {
    SetParseTypeName("DeleteCommand");
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [ " BadName" ])", "Invalid model name");
    TestValid(R"(model_names: ["Box"])");
    TestDesc(R"(model_names: ["Box"])",    R"(Deleted Model "Box")");
    TestDesc(R"(model_names: ["A", "B"])",   "Deleted 2 Models");
}

TEST_F(OtherCommandTest, PasteCommand) {
    auto pc = Command::CreateCommand<PasteCommand>();
    EXPECT_TRUE(pc->GetParentName().empty());

    // PasteCommand requires SetModelsForDescription() to be called before a
    // valid description is available.
    TEST_THROW(pc->GetDescription(), AssertException,
               "SetModelsForDescription() was never called");

    auto a = Model::CreateModel<BoxModel>("A");
    auto b = Model::CreateModel<BoxModel>("B");
    pc->SetModelsForDescription(std::vector<ModelPtr>{ a });
    EXPECT_EQ(R"(Pasted Model "A")", pc->GetDescription());

    pc->SetModelsForDescription(std::vector<ModelPtr>{ a, b });
    EXPECT_EQ("Pasted 2 Models", pc->GetDescription());

    pc->SetParentName("SomeParent");
    EXPECT_EQ(R"(Pasted 2 Models into Model "SomeParent")",
              pc->GetDescription());
}
