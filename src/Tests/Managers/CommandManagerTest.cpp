#include "Commands/CommandList.h"
#include "Managers/CommandManager.h"
#include "Tests/Commands/TestCommand.h"
#include "Tests/TestBaseWithTypes.h"
#include "Tests/Testing2.h"
#include "Util/Assert.h"

// ----------------------------------------------------------------------------
// CommandManagerTest class.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class CommandManagerTest : public TestBaseWithTypes {
  protected:
    bool   expected_is_validating = false;
    StrVec do_names, undo_names;

    CommandManagerTest();

    void InitManager(CommandManager &cm);
};

CommandManagerTest::CommandManagerTest() {
    Parser::Registry::AddType<TestCommand>("TestCommand");
}

void CommandManagerTest::InitManager(CommandManager &cm) {
    // Install a function to process a TestCommand instance.
    auto func = [&](Command &cmd, Command::Op op){
        EXPECT_EQ(expected_is_validating, cm.IsValidating());
        if (op == Command::Op::kDo)
            do_names.push_back(cmd.GetName());
        else
            undo_names.push_back(cmd.GetName());
    };

    cm.RegisterFunction("TestCommand", func);
}

// ----------------------------------------------------------------------------
// CommandManagerTest tests.
// ----------------------------------------------------------------------------

TEST_F(CommandManagerTest, Defaults) {
    CommandManager cm;
    EXPECT_NOT_NULL(cm.GetCommandList());
    EXPECT_EQ(0U, cm.GetCommandList()->GetCommandCount());

    auto &ss = cm.GetSessionState();
    EXPECT_NOT_NULL(ss);
    EXPECT_FALSE(ss->IsPointTargetVisible());
    EXPECT_FALSE(ss->IsEdgeTargetVisible());
    EXPECT_FALSE(ss->AreEdgesShown());
    EXPECT_FALSE(ss->IsBuildVolumeVisible());
    EXPECT_FALSE(ss->IsAxisAligned());
}

TEST_F(CommandManagerTest, ProcessCommandList) {
    CommandManager cm;
    InitManager(cm);

    expected_is_validating = true;  // ProcessCommandList() validates.

    auto cl = Parser::Registry::CreateObject<CommandList>();
    auto tc0 = TestCommand::Create("TC0");
    auto tc1 = TestCommand::Create("TC1");
    cl->AddCommand(tc0);
    cl->AddCommand(tc1);
    cm.ProcessCommandList(cl);

    // Both TestCommands should have been processed.
    EXPECT_EQ((StrVec{ "TC0", "TC1" }), do_names);
    EXPECT_TRUE(undo_names.empty());

    // Should have added both commands to the internal CommandList.
    EXPECT_NOT_NULL(cm.GetCommandList());
    auto &mcl = *cm.GetCommandList();
    EXPECT_EQ(2U,  mcl.GetCommandCount());
    EXPECT_EQ(tc0, mcl.GetCommand(0));
    EXPECT_EQ(tc1, mcl.GetCommand(1));
    EXPECT_EQ(2U,  mcl.GetCurrentIndex());

    // Repeat but with the second command undone.
    do_names.clear();
    undo_names.clear();
    cm.ResetCommandList();
    cl->ProcessUndo();
    cm.ProcessCommandList(cl);
    EXPECT_EQ((StrVec{ "TC0", "TC1" }), do_names);
    EXPECT_EQ((StrVec{ "TC1"        }), undo_names);
    EXPECT_EQ(2U,  mcl.GetCommandCount());
    EXPECT_EQ(tc0, mcl.GetCommand(0));
    EXPECT_EQ(tc1, mcl.GetCommand(1));
    EXPECT_EQ(1U,  mcl.GetCurrentIndex());  // Because of undo.
}


TEST_F(CommandManagerTest, ProcessCommandListWithOrphans) {
    CommandManager cm;
    InitManager(cm);

    expected_is_validating = true;  // ProcessCommandList() validates.

    auto cl = Parser::Registry::CreateObject<CommandList>();
    auto tc0 = TestCommand::Create("TC0");
    auto tc1 = TestCommand::Create("TC1");
    auto tc2 = TestCommand::Create("TC2");
    tc1->SetShouldBeAddedAsOrphan(true);
    cl->AddCommand(tc0);
    cl->AddCommand(tc1);
    cl->ProcessUndo();
    cl->AddCommand(tc2);   // tc1 should now be an orphan in tc2.
    EXPECT_EQ(1U,  tc2->GetOrphanedCommands().size());
    EXPECT_EQ(tc1, tc2->GetOrphanedCommands()[0]);

    // Processing the CommandList should execute and undo orphaned tc1.
    cm.ProcessCommandList(cl);
    EXPECT_EQ((StrVec{ "TC0", "TC1", "TC2" }), do_names);
    EXPECT_EQ((StrVec{ "TC1"        }), undo_names);
    auto &mcl = *cm.GetCommandList();
    EXPECT_EQ(2U,  mcl.GetCommandCount());
    EXPECT_EQ(tc0, mcl.GetCommand(0));
    EXPECT_EQ(tc2, mcl.GetCommand(1));
    EXPECT_EQ(2U,  mcl.GetCurrentIndex());
}

TEST_F(CommandManagerTest, UndoRedo) {
    CommandManager cm;
    InitManager(cm);

    expected_is_validating = false;  // Regular command processing.

    EXPECT_NOT_NULL(cm.GetCommandList());
    auto &mcl = *cm.GetCommandList();

    auto tc0 = TestCommand::Create("TC0");
    auto tc1 = TestCommand::Create("TC1");
    auto tc2 = TestCommand::Create("TC2");

    EXPECT_FALSE(cm.CanUndo());
    EXPECT_FALSE(cm.CanRedo());
    TEST_ASSERT(cm.GetLastCommand(), "CanUndo");

    cm.AddAndDo(tc0);
    EXPECT_EQ((StrVec{ "TC0" }), do_names);
    EXPECT_EQ((StrVec{       }), undo_names);
    EXPECT_EQ(tc0,  cm.GetLastCommand());
    EXPECT_EQ(1U,   mcl.GetCommandCount());
    EXPECT_EQ(1U,   mcl.GetCurrentIndex());
    EXPECT_TRUE(cm.CanUndo());
    EXPECT_FALSE(cm.CanRedo());

    cm.AddAndDo(tc1);
    EXPECT_EQ((StrVec{ "TC0", "TC1" }), do_names);
    EXPECT_EQ((StrVec{              }), undo_names);
    EXPECT_EQ(tc1, cm.GetLastCommand());
    EXPECT_EQ(2U,  mcl.GetCommandCount());
    EXPECT_EQ(2U,  mcl.GetCurrentIndex());
    EXPECT_TRUE(cm.CanUndo());
    EXPECT_FALSE(cm.CanRedo());

    cm.Undo();
    EXPECT_EQ((StrVec{ "TC0", "TC1" }), do_names);
    EXPECT_EQ((StrVec{ "TC1"        }), undo_names);
    EXPECT_EQ(tc0, cm.GetLastCommand());
    EXPECT_EQ(2U,  mcl.GetCommandCount());
    EXPECT_EQ(1U,  mcl.GetCurrentIndex());
    EXPECT_TRUE(cm.CanUndo());
    EXPECT_TRUE(cm.CanRedo());

    cm.Redo();
    EXPECT_EQ((StrVec{ "TC0", "TC1", "TC1" }), do_names);
    EXPECT_EQ((StrVec{ "TC1"               }), undo_names);
    EXPECT_EQ(tc1, cm.GetLastCommand());
    EXPECT_EQ(2U,  mcl.GetCommandCount());
    EXPECT_EQ(2U,  mcl.GetCurrentIndex());
    EXPECT_TRUE(cm.CanUndo());
    EXPECT_FALSE(cm.CanRedo());

    cm.UndoAndPurge();  // Should get rid of tc1.
    EXPECT_EQ((StrVec{ "TC0", "TC1", "TC1" }), do_names);
    EXPECT_EQ((StrVec{ "TC1", "TC1",       }), undo_names);
    EXPECT_EQ(tc0, cm.GetLastCommand());
    EXPECT_EQ(1U,  mcl.GetCommandCount());
    EXPECT_EQ(1U,  mcl.GetCurrentIndex());
    EXPECT_TRUE(cm.CanUndo());
    EXPECT_FALSE(cm.CanRedo());

    // This executes the command but does not change CommandManager state.
    cm.SimulateDo(tc2);
    EXPECT_EQ((StrVec{ "TC0", "TC1", "TC1", "TC2" }), do_names);
    EXPECT_EQ((StrVec{ "TC1", "TC1",              }), undo_names);
    EXPECT_EQ(tc0, cm.GetLastCommand());
    EXPECT_EQ(1U,  mcl.GetCommandCount());
    EXPECT_EQ(1U,  mcl.GetCurrentIndex());
    EXPECT_TRUE(cm.CanUndo());
    EXPECT_FALSE(cm.CanRedo());
}
