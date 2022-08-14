#include "Commands/Command.h"
#include "Commands/CommandList.h"
#include "Tests/SceneTestBase.h"
#include "Testing.h"

DECL_SHARED_PTR(DummyCommand);

// Dummy Command class for testing.
class DummyCommand : public Command {
  public:
    virtual std::string GetDescription() const override { return "DUMMY!"; }

  protected:
    DummyCommand() {}

  private:
    friend class Parser::Registry;
};

class CommandListTest : public SceneTestBase {
  protected:
    CommandListTest() {
        Parser::Registry::AddType<DummyCommand>("DummyCommand");
    }

    DummyCommandPtr CreateDummyCommand() {
        return Parser::Registry::CreateObject<DummyCommand>();
    }
};

TEST_F(CommandListTest, AddRemoveCommand) {
    auto cl = Parser::Registry::CreateObject<CommandList>();
    EXPECT_EQ(0U, cl->GetCommandCount());
    EXPECT_EQ(0U, cl->GetCurrentIndex());

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
