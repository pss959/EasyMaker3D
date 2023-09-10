#include "Panels/Board.h"
#include "Panels/HelpPanel.h"
#include "SG/Search.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"
#include "Util/Assert.h"

/// \ingroup Tests
class BoardTest : public PanelTestBase {
  protected:
    BoardPtr     board;
    HelpPanelPtr panel;

    void InitBoard(const Str &contents = "");
};

void BoardTest::InitBoard(const Str &contents) {
    // Special handling necessary to set up the Board and a HelpPanel.
    const Str s = R"(
  children: [
    Node {
      TEMPLATES: [
        <"nodes/templates/Frame.emd">,
        <"nodes/templates/Board.emd">
      ],
      children: [
        <"nodes/templates/RadialMenu.emd">, # Required for RadialMenuPanel
        <"nodes/Panels.emd">,
        CLONE "T_Board" "TestBoard" {
)" + contents + R"(
        }
      ]
    }
  ]
)";
    board = ReadRealNode<Board>(s, "TestBoard");
    panel = SG::FindTypedNodeInScene<HelpPanel>(*GetScene(), "HelpPanel");

    StoreContext();
}

TEST_F(BoardTest, Defaults) {
    InitBoard();
    EXPECT_EQ(Board::Behavior::kReplaces, board->GetBehavior());
    EXPECT_FALSE(board->IsFloating());
    EXPECT_EQ(Vector3f::Zero(), board->GetTouchOffset());
    EXPECT_NULL(board->GetCurrentPanel());
    EXPECT_FALSE(board->IsShown());
}

TEST_F(BoardTest, Fields) {
    InitBoard(R"(behavior: "kAugments", is_floating: True,)"
              " touch_offset: 1 2 3");
    EXPECT_EQ(Board::Behavior::kAugments, board->GetBehavior());
    EXPECT_TRUE(board->IsFloating());
    EXPECT_EQ(Vector3f(1, 2, 3), board->GetTouchOffset());
}

TEST_F(BoardTest, Replace) {
    InitBoard();

    auto panel0 = GetContext().board_agent->GetPanel("HelpPanel");
    auto panel1 = GetContext().board_agent->GetPanel("HelpPanel");

    Str result;
    auto result_func = [&](const Str &res){ result = res; };

    board->SetPanel(panel0, result_func);
    EXPECT_EQ(panel0, board->GetCurrentPanel());

    board->PushPanel(panel1, result_func);
    EXPECT_EQ(panel1, board->GetCurrentPanel());

    EXPECT_TRUE(board->PopPanel("Result0"));
    EXPECT_EQ("Result0", result);

    EXPECT_FALSE(board->PopPanel("Result1"));
    EXPECT_EQ("Result1", result);
}

TEST_F(BoardTest, Permanent) {
    InitBoard(R"(behavior: "kPermanent")");

    auto panel0 = GetContext().board_agent->GetPanel("HelpPanel");
    auto panel1 = GetContext().board_agent->GetPanel("HelpPanel");

    board->SetPanel(panel0);
    EXPECT_EQ(panel0, board->GetCurrentPanel());

    // Cannot push on a permanent Board.
    TEST_THROW(board->PushPanel(panel1, nullptr),
               AssertException, "kPermanent");
    EXPECT_EQ(panel0, board->GetCurrentPanel());

    // PopPanel() should also assert.
    TEST_THROW(board->PopPanel("SomeResult"), AssertException, "kPermanent");
}
