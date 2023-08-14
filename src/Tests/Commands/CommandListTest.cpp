#include "Commands/Command.h"
#include "Commands/CommandList.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/FilePath.h"

DECL_SHARED_PTR(DummyCommand);

/// Dummy derived Command class for testing.
/// \ingroup Tests
class DummyCommand : public Command {
  public:
    using Command::FixPath;  // Make this callable.

    /// Sets a flag indicating whether the command should be added as an orphan.
    void SetShouldBeAddedAsOrphan(bool b) { orphan_ = b; }

    /// Sets a flag indicating whether the command has undo/redo effect.
    void SetHasUndoRedoEffect(bool b) { undo_redo_ = b; }

    virtual bool HasUndoEffect()         const override { return undo_redo_; }
    virtual bool HasRedoEffect()         const override { return undo_redo_; }
    virtual bool ShouldBeAddedAsOrphan() const override { return orphan_;    }
    virtual std::string GetDescription() const override { return "DUMMY!";   }
  protected:
    DummyCommand() {}
  private:
    bool undo_redo_ = true;   ///< For testing undo/redo effects.
    bool orphan_    = false;  ///< For testing orphaned commands.
    friend class Parser::Registry;
};

/// \ingroup Tests
class CommandListTest : public SceneTestBase {
  protected:
    CommandListTest() {
        Parser::Registry::AddType<DummyCommand>("DummyCommand");
    }
    DummyCommandPtr CreateDummyCommand() {
        return Parser::Registry::CreateObject<DummyCommand>();
    }
};

TEST_F(CommandListTest, Defaults) {
    auto cl = Parser::Registry::CreateObject<CommandList>();
    EXPECT_NULL(cl->GetAppInfo());
    EXPECT_EQ(0U, cl->GetCommandCount());
    EXPECT_EQ(0U, cl->GetCurrentIndex());
}

TEST_F(CommandListTest, Command) {
    // Use DummyCommand to test basic Command interface.
    auto dc = CreateDummyCommand();
    EXPECT_FALSE(dc->IsFinalized());
    EXPECT_FALSE(dc->IsValidating());
    EXPECT_FALSE(dc->GetSelection().HasAny());
    EXPECT_TRUE(dc->GetOrphanedCommands().empty());
    EXPECT_NULL(dc->GetExecData());
    EXPECT_TRUE(dc->Command::HasUndoEffect());
    EXPECT_TRUE(dc->Command::HasRedoEffect());
    EXPECT_FALSE(dc->Command::ShouldBeAddedAsOrphan());

    dc->SetIsFinalized();
    EXPECT_TRUE(dc->IsFinalized());

    dc->SetIsValidating(true);
    EXPECT_TRUE(dc->IsValidating());

    std::unique_ptr<Command::ExecData> data;
    dc->SetExecData(data.get());
    EXPECT_EQ(data.get(), dc->GetExecData());

    EXPECT_EQ("/a/b/c",                        DummyCommand::FixPath("/a/b/c"));
    EXPECT_EQ(FilePath("a/b/c").GetAbsolute(), DummyCommand::FixPath("a/b/c"));
}

TEST_F(CommandListTest, IsValid) {
    // Valid AppInfo string.
    const std::string app_str = R"(
  app_info: AppInfo { version: "1.0.0", session_state: SessionState {} }
)";

    TestInvalid("CommandList {}", "Missing app_info field");
    TestInvalid("CommandList { " + app_str + ", current_index: 1 }",
                "Invalid current_index");
    TestInvalid("CommandList { " + app_str +
                ", commands: [ PasteCommand {} ], current_index: 2 }",
                "Invalid current_index");
    TestValid("CommandList { " + app_str + ", current_index: 0 }");
}

TEST_F(CommandListTest, AppInfo) {
    auto cl = Parser::Registry::CreateObject<CommandList>();
    cl->SetAppInfo(CreateObject<AppInfo>());
    EXPECT_NOT_NULL(cl->GetAppInfo());
}

TEST_F(CommandListTest, AddRemoveCommand) {
    auto cl = Parser::Registry::CreateObject<CommandList>();

    auto dc1 = CreateDummyCommand();
    cl->AddCommand(dc1);
    EXPECT_EQ(1U,  cl->GetCommandCount());
    EXPECT_EQ(1U,  cl->GetCurrentIndex());
    EXPECT_EQ(dc1, cl->GetCommand(0));

    auto dc2 = CreateDummyCommand();
    cl->AddCommand(dc2);
    EXPECT_EQ(2U,  cl->GetCommandCount());
    EXPECT_EQ(2U,  cl->GetCurrentIndex());
    EXPECT_EQ(dc1, cl->GetCommand(0));
    EXPECT_EQ(dc2, cl->GetCommand(1));

    auto dc3 = CreateDummyCommand();
    cl->AddCommand(dc3);
    EXPECT_EQ(3U,  cl->GetCommandCount());
    EXPECT_EQ(3U,  cl->GetCurrentIndex());
    EXPECT_EQ(dc1, cl->GetCommand(0));
    EXPECT_EQ(dc2, cl->GetCommand(1));
    EXPECT_EQ(dc3, cl->GetCommand(2));

    cl->RemoveLastCommand();
    EXPECT_EQ(2U,  cl->GetCommandCount());
    EXPECT_EQ(2U,  cl->GetCurrentIndex());
    EXPECT_EQ(dc1, cl->GetCommand(0));
    EXPECT_EQ(dc2, cl->GetCommand(1));

    cl->RemoveLastCommand();
    EXPECT_EQ(1U,  cl->GetCommandCount());
    EXPECT_EQ(1U,  cl->GetCurrentIndex());
    EXPECT_EQ(dc1, cl->GetCommand(0));

    cl->Reset();
    EXPECT_EQ(0U, cl->GetCommandCount());
    EXPECT_EQ(0U, cl->GetCurrentIndex());
    EXPECT_NOT_NULL(cl->GetAppInfo());
}

TEST_F(CommandListTest, UndoRedo) {
    auto cl = Parser::Registry::CreateObject<CommandList>();
    EXPECT_FALSE(cl->CanUndo());
    EXPECT_FALSE(cl->CanRedo());

    auto dc1 = CreateDummyCommand();
    cl->AddCommand(dc1);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_FALSE(cl->CanRedo());
    EXPECT_EQ(dc1, cl->GetCommandToUndo());

    auto dc2 = CreateDummyCommand();
    cl->AddCommand(dc2);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_FALSE(cl->CanRedo());
    EXPECT_EQ(dc2, cl->GetCommandToUndo());

    auto c = cl->ProcessUndo();
    EXPECT_EQ(dc2, c);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_TRUE(cl->CanRedo());
    EXPECT_EQ(dc1, cl->GetCommandToUndo());
    EXPECT_EQ(dc2, cl->GetCommandToRedo());

    c = cl->ProcessUndo();
    EXPECT_EQ(dc1, c);
    EXPECT_FALSE(cl->CanUndo());
    EXPECT_TRUE(cl->CanRedo());
    EXPECT_EQ(dc1, cl->GetCommandToRedo());

    c = cl->ProcessRedo();
    EXPECT_EQ(dc1, c);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_TRUE(cl->CanRedo());
    EXPECT_EQ(dc1, cl->GetCommandToUndo());
    EXPECT_EQ(dc2, cl->GetCommandToRedo());

    c = cl->ProcessRedo();
    EXPECT_EQ(dc2, c);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_FALSE(cl->CanRedo());
    EXPECT_EQ(dc2, cl->GetCommandToUndo());

    // Test commands that have no undo/redo effect.
    auto dc3 = CreateDummyCommand();
    dc3->SetHasUndoRedoEffect(false);
    cl->AddCommand(dc3);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_FALSE(cl->CanRedo());
    EXPECT_EQ(dc2, cl->GetCommandToUndo());

    cl->ProcessUndo();
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_TRUE(cl->CanRedo());
    EXPECT_EQ(dc1, cl->GetCommandToUndo());
    EXPECT_EQ(dc2, cl->GetCommandToRedo());

    cl->ProcessRedo();
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_FALSE(cl->CanRedo());
    EXPECT_EQ(dc2, cl->GetCommandToUndo());
}

TEST_F(CommandListTest, Changes) {
    auto cl = Parser::Registry::CreateObject<CommandList>();
    EXPECT_FALSE(cl->DidCommandsChange());
    EXPECT_FALSE(cl->AreAnyChanges());

    auto dc1 = CreateDummyCommand();
    cl->AddCommand(dc1);
    EXPECT_TRUE(cl->DidCommandsChange());
    EXPECT_TRUE(cl->AreAnyChanges());

    auto dc2 = CreateDummyCommand();
    cl->AddCommand(dc2);
    EXPECT_TRUE(cl->DidCommandsChange());
    EXPECT_TRUE(cl->AreAnyChanges());

    EXPECT_EQ(2U,  cl->GetCommandCount());
    EXPECT_EQ(2U,  cl->GetCurrentIndex());
    EXPECT_EQ(dc1, cl->GetCommand(0));
    EXPECT_EQ(dc2, cl->GetCommand(1));

    cl->ClearChanges();
    EXPECT_EQ(2U,  cl->GetCommandCount());
    EXPECT_EQ(2U,  cl->GetCurrentIndex());
    EXPECT_EQ(dc1, cl->GetCommand(0));
    EXPECT_EQ(dc2, cl->GetCommand(1));
    EXPECT_FALSE(cl->DidCommandsChange());
    EXPECT_FALSE(cl->AreAnyChanges());

    auto dc3 = CreateDummyCommand();
    cl->AddCommand(dc3);
    EXPECT_EQ(3U,  cl->GetCommandCount());
    EXPECT_EQ(3U,  cl->GetCurrentIndex());
    EXPECT_EQ(dc1, cl->GetCommand(0));
    EXPECT_EQ(dc2, cl->GetCommand(1));
    EXPECT_EQ(dc3, cl->GetCommand(2));
    EXPECT_TRUE(cl->DidCommandsChange());
    EXPECT_TRUE(cl->AreAnyChanges());

    auto c = cl->ProcessUndo();
    cl->ClearChanges();
    EXPECT_EQ(3U,  cl->GetCommandCount());
    EXPECT_EQ(2U,  cl->GetCurrentIndex());
    EXPECT_EQ(dc1, cl->GetCommand(0));
    EXPECT_EQ(dc2, cl->GetCommand(1));
    EXPECT_EQ(dc3, cl->GetCommand(2));
    EXPECT_EQ(dc3, c);
    EXPECT_TRUE(cl->CanUndo());
    EXPECT_TRUE(cl->CanRedo());
    EXPECT_EQ(dc2, cl->GetCommandToUndo());
    EXPECT_EQ(dc3, cl->GetCommandToRedo());
    EXPECT_FALSE(cl->DidCommandsChange());
    EXPECT_FALSE(cl->AreAnyChanges());
}

TEST_F(CommandListTest, NonOrphans) {
    auto cl = Parser::Registry::CreateObject<CommandList>();

    auto dc1 = CreateDummyCommand();
    cl->AddCommand(dc1);

    auto dc2 = CreateDummyCommand();
    dc2->SetShouldBeAddedAsOrphan(false);
    cl->AddCommand(dc2);

    // Undo should not cause dc2 to become an orphan.
    cl->ProcessUndo();

    // dc2 should not be added as an orphaned command to dc3.
    auto dc3 = CreateDummyCommand();
    cl->AddCommand(dc3);
    EXPECT_TRUE(dc3->GetOrphanedCommands().empty());
}

TEST_F(CommandListTest, Orphans) {
    auto cl = Parser::Registry::CreateObject<CommandList>();

    auto dc1 = CreateDummyCommand();
    cl->AddCommand(dc1);

    auto dc2 = CreateDummyCommand();
    dc2->SetShouldBeAddedAsOrphan(true);
    cl->AddCommand(dc2);

    // Undo should cause dc2 to become an orphan.
    cl->ProcessUndo();

    // dc2 should be added as an orphaned command to dc3.
    auto dc3 = CreateDummyCommand();
    cl->AddCommand(dc3);
    EXPECT_EQ(1U,  dc3->GetOrphanedCommands().size());
    EXPECT_EQ(dc2, dc3->GetOrphanedCommands()[0]);
}
