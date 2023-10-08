#include "Commands/Command.h"
#include "Commands/CommandList.h"
#include "Tests/Commands/TestCommand.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/FilePath.h"

/// \ingroup Tests
class CommandListTest : public SceneTestBase {
  protected:
    CommandListTest() {
        Parser::Registry::AddType<TestCommand>("TestCommand");
        SetParseTypeName("CommandList");
    }
};

TEST_F(CommandListTest, Defaults) {
    auto cl = Parser::Registry::CreateObject<CommandList>();
    EXPECT_NULL(cl->GetAppInfo());
    EXPECT_EQ(0U, cl->GetCommandCount());
    EXPECT_EQ(0U, cl->GetCurrentIndex());
}

TEST_F(CommandListTest, Command) {
    // Use TestCommand to test basic Command interface.
    auto tc = TestCommand::Create();
    EXPECT_FALSE(tc->IsFinalized());
    EXPECT_FALSE(tc->IsValidating());
    EXPECT_FALSE(tc->GetSelection().HasAny());
    EXPECT_TRUE(tc->GetOrphanedCommands().empty());
    EXPECT_NULL(tc->GetExecData());
    EXPECT_TRUE(tc->Command::HasUndoEffect());
    EXPECT_TRUE(tc->Command::HasRedoEffect());
    EXPECT_FALSE(tc->Command::ShouldBeAddedAsOrphan());

    tc->SetIsFinalized();
    EXPECT_TRUE(tc->IsFinalized());

    tc->SetIsValidating(true);
    EXPECT_TRUE(tc->IsValidating());

    std::unique_ptr<Command::ExecData> data;
    tc->SetExecData(data.get());
    EXPECT_EQ(data.get(), tc->GetExecData());

    EXPECT_EQ("/a/b/c",                        TestCommand::FixPath("/a/b/c"));
    EXPECT_EQ(FilePath("a/b/c").GetAbsolute(), TestCommand::FixPath("a/b/c"));
}

TEST_F(CommandListTest, IsValid) {
    // Valid AppInfo string.
    const Str app_str = R"(
  app_info: AppInfo { version: "1.0.0", session_state: SessionState {} }
)";

    TestInvalid("", "Missing app_info field");
    TestInvalid(app_str + ", current_index: 1", "Invalid current_index");
    TestInvalid(app_str + ", commands: [ PasteCommand {} ], current_index: 2",
                "Invalid current_index");
    TestValid(app_str + ", current_index: 0");
}

TEST_F(CommandListTest, AppInfo) {
    auto cl = Parser::Registry::CreateObject<CommandList>();
    cl->SetAppInfo(CreateObject<AppInfo>());
    EXPECT_NOT_NULL(cl->GetAppInfo());
}

TEST_F(CommandListTest, AddRemoveCommand) {
    auto cl = Parser::Registry::CreateObject<CommandList>();

    auto tc1 = TestCommand::Create();
    cl->AddCommand(tc1);
    EXPECT_EQ(1U,  cl->GetCommandCount());
    EXPECT_EQ(1U,  cl->GetCurrentIndex());
    EXPECT_EQ(tc1, cl->GetCommand(0));

    auto tc2 = TestCommand::Create();
    cl->AddCommand(tc2);
    EXPECT_EQ(2U,  cl->GetCommandCount());
    EXPECT_EQ(2U,  cl->GetCurrentIndex());
    EXPECT_EQ(tc1, cl->GetCommand(0));
    EXPECT_EQ(tc2, cl->GetCommand(1));

    auto tc3 = TestCommand::Create();
    cl->AddCommand(tc3);
    EXPECT_EQ(3U,  cl->GetCommandCount());
    EXPECT_EQ(3U,  cl->GetCurrentIndex());
    EXPECT_EQ(tc1, cl->GetCommand(0));
    EXPECT_EQ(tc2, cl->GetCommand(1));
    EXPECT_EQ(tc3, cl->GetCommand(2));

    cl->RemoveLastCommand();
    EXPECT_EQ(2U,  cl->GetCommandCount());
    EXPECT_EQ(2U,  cl->GetCurrentIndex());
    EXPECT_EQ(tc1, cl->GetCommand(0));
    EXPECT_EQ(tc2, cl->GetCommand(1));

    cl->RemoveLastCommand();
    EXPECT_EQ(1U,  cl->GetCommandCount());
    EXPECT_EQ(1U,  cl->GetCurrentIndex());
    EXPECT_EQ(tc1, cl->GetCommand(0));

    cl->Reset();
    EXPECT_EQ(0U, cl->GetCommandCount());
    EXPECT_EQ(0U, cl->GetCurrentIndex());
    EXPECT_NOT_NULL(cl->GetAppInfo());
}

TEST_F(CommandListTest, UndoRedo) {
    auto cl = Parser::Registry::CreateObject<CommandList>();
    EXPECT_FALSE(cl->CanUndo());
    EXPECT_FALSE(cl->CanRedo());

    auto tc1 = TestCommand::Create();
    cl->AddCommand(tc1);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_FALSE(cl->CanRedo());
    EXPECT_EQ(tc1, cl->GetCommandToUndo());

    auto tc2 = TestCommand::Create();
    cl->AddCommand(tc2);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_FALSE(cl->CanRedo());
    EXPECT_EQ(tc2, cl->GetCommandToUndo());

    auto c = cl->ProcessUndo();
    EXPECT_EQ(tc2, c);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_TRUE(cl->CanRedo());
    EXPECT_EQ(tc1, cl->GetCommandToUndo());
    EXPECT_EQ(tc2, cl->GetCommandToRedo());

    c = cl->ProcessUndo();
    EXPECT_EQ(tc1, c);
    EXPECT_FALSE(cl->CanUndo());
    EXPECT_TRUE(cl->CanRedo());
    EXPECT_EQ(tc1, cl->GetCommandToRedo());

    c = cl->ProcessRedo();
    EXPECT_EQ(tc1, c);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_TRUE(cl->CanRedo());
    EXPECT_EQ(tc1, cl->GetCommandToUndo());
    EXPECT_EQ(tc2, cl->GetCommandToRedo());

    c = cl->ProcessRedo();
    EXPECT_EQ(tc2, c);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_FALSE(cl->CanRedo());
    EXPECT_EQ(tc2, cl->GetCommandToUndo());

    // Test commands that have no undo/redo effect.
    auto tc3 = TestCommand::Create();
    tc3->SetHasUndoRedoEffect(false);
    cl->AddCommand(tc3);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_FALSE(cl->CanRedo());
    EXPECT_EQ(tc2, cl->GetCommandToUndo());

    cl->ProcessUndo();
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_TRUE(cl->CanRedo());
    EXPECT_EQ(tc1, cl->GetCommandToUndo());
    EXPECT_EQ(tc2, cl->GetCommandToRedo());

    cl->ProcessRedo();
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_FALSE(cl->CanRedo());
    EXPECT_EQ(tc2, cl->GetCommandToUndo());
}

TEST_F(CommandListTest, Changes) {
    auto cl = Parser::Registry::CreateObject<CommandList>();
    EXPECT_FALSE(cl->DidCommandsChange());
    EXPECT_FALSE(cl->AreAnyChanges());

    auto tc1 = TestCommand::Create();
    cl->AddCommand(tc1);
    EXPECT_TRUE(cl->DidCommandsChange());
    EXPECT_TRUE(cl->AreAnyChanges());

    auto tc2 = TestCommand::Create();
    cl->AddCommand(tc2);
    EXPECT_TRUE(cl->DidCommandsChange());
    EXPECT_TRUE(cl->AreAnyChanges());

    EXPECT_EQ(2U,  cl->GetCommandCount());
    EXPECT_EQ(2U,  cl->GetCurrentIndex());
    EXPECT_EQ(tc1, cl->GetCommand(0));
    EXPECT_EQ(tc2, cl->GetCommand(1));

    cl->ClearChanges();
    EXPECT_EQ(2U,  cl->GetCommandCount());
    EXPECT_EQ(2U,  cl->GetCurrentIndex());
    EXPECT_EQ(tc1, cl->GetCommand(0));
    EXPECT_EQ(tc2, cl->GetCommand(1));
    EXPECT_FALSE(cl->DidCommandsChange());
    EXPECT_FALSE(cl->AreAnyChanges());

    auto tc3 = TestCommand::Create();
    cl->AddCommand(tc3);
    EXPECT_EQ(3U,  cl->GetCommandCount());
    EXPECT_EQ(3U,  cl->GetCurrentIndex());
    EXPECT_EQ(tc1, cl->GetCommand(0));
    EXPECT_EQ(tc2, cl->GetCommand(1));
    EXPECT_EQ(tc3, cl->GetCommand(2));
    EXPECT_TRUE(cl->DidCommandsChange());
    EXPECT_TRUE(cl->AreAnyChanges());

    auto c = cl->ProcessUndo();
    cl->ClearChanges();
    EXPECT_EQ(3U,  cl->GetCommandCount());
    EXPECT_EQ(2U,  cl->GetCurrentIndex());
    EXPECT_EQ(tc1, cl->GetCommand(0));
    EXPECT_EQ(tc2, cl->GetCommand(1));
    EXPECT_EQ(tc3, cl->GetCommand(2));
    EXPECT_EQ(tc3, c);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_TRUE(cl->CanRedo());
    EXPECT_EQ(tc2, cl->GetCommandToUndo());
    EXPECT_EQ(tc3, cl->GetCommandToRedo());
    EXPECT_FALSE(cl->DidCommandsChange());
    EXPECT_FALSE(cl->AreAnyChanges());
}

TEST_F(CommandListTest, NonOrphans) {
    auto cl = Parser::Registry::CreateObject<CommandList>();

    auto tc1 = TestCommand::Create();
    cl->AddCommand(tc1);

    auto tc2 = TestCommand::Create();
    tc2->SetShouldBeAddedAsOrphan(false);
    cl->AddCommand(tc2);

    // Undo should not cause tc2 to become an orphan.
    cl->ProcessUndo();

    // tc2 should not be added as an orphaned command to tc3.
    auto tc3 = TestCommand::Create();
    cl->AddCommand(tc3);
    EXPECT_TRUE(tc3->GetOrphanedCommands().empty());
}

TEST_F(CommandListTest, Orphans) {
    auto cl = Parser::Registry::CreateObject<CommandList>();

    auto tc1 = TestCommand::Create();
    cl->AddCommand(tc1);

    auto tc2 = TestCommand::Create();
    tc2->SetShouldBeAddedAsOrphan(true);
    cl->AddCommand(tc2);

    // Undo should cause tc2 to become an orphan.
    cl->ProcessUndo();

    // tc2 should be added as an orphaned command to tc3.
    auto tc3 = TestCommand::Create();
    cl->AddCommand(tc3);
    EXPECT_EQ(1U,  tc3->GetOrphanedCommands().size());
    EXPECT_EQ(tc2, tc3->GetOrphanedCommands()[0]);
}
