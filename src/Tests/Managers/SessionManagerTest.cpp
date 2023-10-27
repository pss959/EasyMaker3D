#include "Agents/ActionAgent.h"
#include "Commands/CommandList.h"
#include "IO/STLReader.h"
#include "Items/SessionState.h"
#include "Items/UnitConversion.h"
#include "Managers/CommandManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "Math/MeshUtils.h"
#include "Models/RootModel.h"
#include "Tests/Commands/TestCommand.h"
#include "Tests/SelectionTestBase.h"
#include "Tests/TempFile.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class SessionManagerTest : public SelectionTestBase {
  protected:
    class TestActionManager : public ActionAgent {
        virtual void Reset() override {}
        virtual void UpdateFromSessionState(
            const SessionState &state) override {}
        virtual bool CanApplyAction(Action action) const { return false; }
        virtual void ApplyAction(Action action) override {}
    };
    DECL_SHARED_PTR(TestActionManager);

    TestActionManagerPtr am;
    CommandManagerPtr    cm;
    SelectionManagerPtr  sm;
    SessionManagerPtr    ssm;

    /// The constructor sets up the managers.
    SessionManagerTest() {
        Parser::Registry::AddType<TestCommand>("TestCommand");

        am.reset(new TestActionManager);
        cm.reset(new CommandManager);
        sm.reset(new SelectionManager);
        ssm.reset(new SessionManager(am, cm, sm, FilePath()));

        // Install a function to process a TestCommand instance.
        auto func = [&](Command &cmd, Command::Op op){};
        cm->RegisterFunction("TestCommand", func);

        // SelectionManager requires a RootModel for most operations.
        sm->SetRootModel(root);
    }
};

TEST_F(SessionManagerTest, Defaults) {
    EXPECT_FALSE(ssm->GetModifications().HasAny());
    EXPECT_FALSE(ssm->CanSaveSession());
    EXPECT_EQ("<Untitled Session>", ssm->GetSessionString());
    EXPECT_EQ("",                   ssm->GetModelNameForExport());
    EXPECT_EQ("",                   ssm->GetPreviousSessionName());
    EXPECT_EQ("",                   ssm->GetCurrentSessionName());
}

TEST_F(SessionManagerTest, SessionString) {
    // Execute a command that changes the scene.
    cm->AddAndDo(TestCommand::Create());
    EXPECT_EQ("kScene|kCommands", ssm->GetModifications().ToString());
    EXPECT_TRUE(ssm->CanSaveSession());
    EXPECT_EQ("<Untitled Session> [*+]", ssm->GetSessionString());

    // Change the session state.
    cm->GetSessionState()->SetAxisAligned(true);
    EXPECT_EQ("kScene|kSessionState|kCommands",
              ssm->GetModifications().ToString());
    EXPECT_TRUE(ssm->CanSaveSession());
    EXPECT_EQ("<Untitled Session> [*!+]", ssm->GetSessionString());

    // Start a new session to clear everything.
    ssm->NewSession();
    EXPECT_FALSE(ssm->GetModifications().HasAny());
    EXPECT_FALSE(ssm->CanSaveSession());
    EXPECT_EQ("<Untitled Session>", ssm->GetSessionString());
}

TEST_F(SessionManagerTest, SaveSession) {
    // Execute two commands.
    auto cmd0 = TestCommand::Create();
    auto cmd1 = TestCommand::Create();
    cmd0->SetText("First");
    cmd1->SetText("Second");

    cm->AddAndDo(cmd0);
    cm->AddAndDo(cmd1);

    // Test failure first so changes are not cleared.
    EXPECT_FALSE(ssm->SaveSession("/no/such/path.ems"));

    TempFile tmp("");
    StrVec comments{ "Comment line 1", "Comment line 2" };
    EXPECT_TRUE(ssm->SaveSessionWithComments(tmp.GetPath(), comments));

    const Str exp =
R"(# Comment line 1
# Comment line 2
CommandList {
  app_info: AppInfo {
    version: ")" + TK::kVersionString + R"(",
    session_state: SessionState {
    },
  },
  commands: [
    TestCommand {
      text: "First",
    },
    TestCommand {
      text: "Second",
    },
  ],
  current_index: 2,
}
)";
    EXPECT_EQ(exp, tmp.GetContents());
}

TEST_F(SessionManagerTest, LoadSession) {
    // Opposite of SaveSession test.
    const Str input =
R"(# A comment.
CommandList {
  app_info: AppInfo {
    version: ")" + TK::kVersionString + R"(",
    session_state: SessionState {},
  },
  commands: [
    TestCommand { text: "First"  },
    TestCommand { text: "Second" },
  ],
  current_index: 2,
}
)";

    {
        TempFile tmp(input);
        Str error;
        EXPECT_TRUE(ssm->LoadSession(tmp.GetPath(), error));
        EXPECT_EQ(2U, cm->GetCommandList()->GetCommandCount());
        auto &cl = *cm->GetCommandList();
        auto cmd0 = std::dynamic_pointer_cast<TestCommand>(cl.GetCommand(0));
        auto cmd1 = std::dynamic_pointer_cast<TestCommand>(cl.GetCommand(1));
        EXPECT_NOT_NULL(cmd0);
        EXPECT_NOT_NULL(cmd1);
        EXPECT_EQ("First",  cmd0->GetText());
        EXPECT_EQ("Second", cmd1->GetText());
        EXPECT_EQ(2U,       cl.GetCurrentIndex());
    }

    // Test errors.
    {
        TempFile tmp("BoxModel {}");
        Str error;
        EXPECT_FALSE(ssm->LoadSession(tmp.GetPath(), error));
        EXPECT_TRUE(error.contains("Expected a CommandList"));
    }
    {
        TempFile tmp("Some Garbage");
        Str error;
        EXPECT_FALSE(ssm->LoadSession(tmp.GetPath(), error));
        EXPECT_TRUE(error.contains("Unknown object type"));
    }
}

TEST_F(SessionManagerTest, Export) {
    // Need a real selected Model. Change the scale of the Box to test units.
    box0->SetScale(Vector3f(2, 3, 4));
    sm->ChangeModelSelection(BuildSelPath(ModelVec{ root, par0, box0 }), false);
    EXPECT_EQ("Box0", ssm->GetModelNameForExport());

    TempFile tmp("");
    EXPECT_TRUE(ssm->Export(tmp.GetPath(), FileFormat::kTextSTL,
                            *CreateObject<UnitConversion>()));

    // Read the results and test the resulting TriMesh.
    Str error;
    const auto mesh = ReadSTLFile(tmp.GetPath(), 1, error);
    EXPECT_EQ(8U,  mesh.points.size());
    EXPECT_EQ(12U, mesh.GetTriangleCount());
    const auto bounds = ComputeMeshBounds(mesh);
    EXPECT_EQ(Point3f(0, 0, 0),  bounds.GetCenter());
    EXPECT_EQ(Vector3f(4, 6, 8), bounds.GetSize());
}
