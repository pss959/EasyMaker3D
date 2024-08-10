#include "Managers/BoardManager.h"
#include "Managers/PanelManager.h"
#include "Math/Frustum.h"
#include "Panels/Board.h"
#include "Panels/HelpPanel.h"
#include "SG/Search.h"
#include "SG/WindowCamera.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class BoardManagerTest : public SceneTestBase {
  protected:
    BoardManagerPtr bm;

    /// The constructor sets up a BoardManager with a Frustum, all Panels, and
    /// some Board instances.
    BoardManagerTest();
};

BoardManagerTest::BoardManagerTest() {
    const Str contents = R"(
  children: [
    Node {
      TEMPLATES: [
        <"nodes/templates/Frame.emd">,
        <"nodes/templates/Board.emd">
      ],
      children: [
        <"nodes/templates/RadialMenu.emd">, # Required for RadialMenuPanel
        <"nodes/Panels.emd">,
        CLONE "T_Board" "TB0" { is_floating: True, behavior: "kReplaces" },
        CLONE "T_Board" "TB1" { is_floating: True, behavior: "kReplaces" },
        CLONE "T_Board" "TB2" { is_floating: True, behavior: "kAugments" },
      ]
    }
  ]
)";

    // Read a Scene that sets up all known Panels.
    auto scene = ReadRealScene(contents);

    PanelManagerPtr pm(new PanelManager);
    Panel::ContextPtr context(new Panel::Context);  // Contents do not matter.
    pm->FindAllPanels(*scene, context);

    bm.reset(new BoardManager(pm));

    // Store a Frustum.
    auto cam = CreateObject<SG::WindowCamera>();
    cam->SetPosition(Point3f(0, 0, 40));
    FrustumPtr frustum(new Frustum);
    cam->BuildFrustum(Vector2ui(800, 600), *frustum);
    bm->SetFrustum(frustum);
}

TEST_F(BoardManagerTest, GetPanel) {
    EXPECT_NULL(bm->GetCurrentBoard());
    EXPECT_NOT_NULL(bm->GetPanel("HelpPanel"));
    EXPECT_NOT_NULL(bm->GetTypedPanel<HelpPanel>("HelpPanel"));
}

TEST_F(BoardManagerTest, Show) {
    auto board = SG::FindTypedNodeInScene<Board>(*GetScene(), "TB0");

    EXPECT_FALSE(board->IsShown());
    EXPECT_NULL(bm->GetCurrentBoard());
    bm->ShowBoard(board, true);
    EXPECT_TRUE(board->IsShown());
    EXPECT_EQ(board, bm->GetCurrentBoard());
    bm->ShowBoard(board, false);
    EXPECT_FALSE(board->IsShown());
    EXPECT_NULL(bm->GetCurrentBoard());

    // Test Reset().
    bm->ShowBoard(board, true);
    bm->Reset();
    EXPECT_NULL(bm->GetCurrentBoard());
}

TEST_F(BoardManagerTest, PushAndClosePanels) {
    auto board0 = SG::FindTypedNodeInScene<Board>(*GetScene(), "TB0");
    auto board1 = SG::FindTypedNodeInScene<Board>(*GetScene(), "TB1");
    auto board2 = SG::FindTypedNodeInScene<Board>(*GetScene(), "TB2");

    Str result;
    auto result_func = [&](const Str &res){ result = res; };

    auto panel0 = bm->GetPanel("HelpPanel");
    auto panel1 = bm->GetPanel("DialogPanel");

    board0->SetPanel(panel0, result_func);
    EXPECT_EQ(panel0, board0->GetCurrentPanel());
    bm->ShowBoard(board0, true);
    EXPECT_EQ(board0, bm->GetCurrentBoard());

    // board2 augments, so showing it leaves board0 visible.
    board2->SetPanel(panel1, result_func);
    bm->ShowBoard(board2, true);
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(board2, bm->GetCurrentBoard());
    EXPECT_EQ(panel0, board0->GetCurrentPanel());
    EXPECT_EQ(panel1, board2->GetCurrentPanel());
    EXPECT_TRUE(board0->IsShown());
    EXPECT_TRUE(board2->IsShown());

    // Hiding board2 should not affect board0.
    bm->ShowBoard(board2, false);
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(board0, bm->GetCurrentBoard());
    EXPECT_EQ(panel0, board0->GetCurrentPanel());
    EXPECT_TRUE(board0->IsShown());
    EXPECT_FALSE(board2->IsShown());

    bm->PushPanel(panel1, result_func);
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(panel1, board0->GetCurrentPanel());

    bm->ClosePanel("Tested");
    EXPECT_EQ("Tested", result);
    EXPECT_EQ(panel0, board0->GetCurrentPanel());

    bm->ClosePanel("Finished");
    EXPECT_EQ("Finished", result);
    EXPECT_NULL(board0->GetCurrentPanel());

    // Test board replacement.
    result.clear();
    board0->SetPanel(panel0, result_func);
    EXPECT_EQ(panel0, board0->GetCurrentPanel());
    bm->ShowBoard(board0, true);
    EXPECT_EQ(board0, bm->GetCurrentBoard());

    // board1 replaces, so showing it hides board0 visible.
    board1->SetPanel(panel1, result_func);
    bm->ShowBoard(board1, true);
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(board1, bm->GetCurrentBoard());
    EXPECT_EQ(panel0, board0->GetCurrentPanel());
    EXPECT_EQ(panel1, board1->GetCurrentPanel());
    EXPECT_FALSE(board0->IsShown());
    EXPECT_TRUE(board1->IsShown());

    // Hiding board1 should show board0 again.
    bm->ShowBoard(board1, false);
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(board0, bm->GetCurrentBoard());
    EXPECT_EQ(panel0, board0->GetCurrentPanel());
    EXPECT_TRUE(board0->IsShown());
    EXPECT_FALSE(board1->IsShown());
}
