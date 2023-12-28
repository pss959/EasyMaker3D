#include "Base/Event.h"
#include "Handlers/BoardHandler.h"
#include "Panels/Board.h"
#include "Panels/HelpPanel.h"
#include "Parser/Registry.h"
#include "SG/Search.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class BoardHandlerTest : public SceneTestBase {
  protected:
    /// Derived Panel class that tracks calls to HandleEvent().
    class TestPanel : public Panel {
      public:
        size_t event_count = 0;  ///< Tracks calls to HandleEvent().
        virtual bool HandleEvent(const Event &event) {
            ++event_count;
            return true;
        }
      protected:
        TestPanel() {}
        virtual void InitInterface() override {}
        friend class Parser::Registry;
    };
    DECL_SHARED_PTR(TestPanel);

    /// The constructor sets up a TestPanel and 3 Board instances for testing.
    BoardHandlerTest();

    TestPanelPtr panel0, panel1, panel2;
    BoardPtr     board0, board1, board2;
};

BoardHandlerTest::BoardHandlerTest() {
    Parser::Registry::AddType<TestPanel>("TestPanel");

    const Str s = R"(
  children: [
    Node {
      TEMPLATES: [
        <"nodes/templates/Frame.emd">,
        <"nodes/templates/Board.emd">
      ],
      children: [
        TestPanel "TestPanel0" { pane: BoxPane { min_size: 100 100 } },
        TestPanel "TestPanel1" { pane: BoxPane { min_size: 100 100 } },
        TestPanel "TestPanel2" { pane: BoxPane { min_size: 100 100 } },
        CLONE "T_Board" "Board0" {},
        CLONE "T_Board" "Board1" {},
        CLONE "T_Board" "Board2" {},
      ]
    }
  ]
)";
    ReadRealScene(s);
    panel0 = SG::FindTypedNodeInScene<TestPanel>(*GetScene(), "TestPanel0");
    panel1 = SG::FindTypedNodeInScene<TestPanel>(*GetScene(), "TestPanel1");
    panel2 = SG::FindTypedNodeInScene<TestPanel>(*GetScene(), "TestPanel2");
    board0 = SG::FindTypedNodeInScene<Board>(*GetScene(), "Board0");
    board1 = SG::FindTypedNodeInScene<Board>(*GetScene(), "Board1");
    board2 = SG::FindTypedNodeInScene<Board>(*GetScene(), "Board2");

    board0->SetPanel(panel0);
    board1->SetPanel(panel1);
    board2->SetPanel(panel2);
}

TEST_F(BoardHandlerTest, Boards) {
    Event event;  // Contents do not matter.

    BoardHandler bh;
    EXPECT_FALSE(bh.IsEnabled());
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled, bh.HandleEvent(event));

    bh.AddBoard(board0);
    bh.AddBoard(board1);
    bh.AddBoard(board2);
    EXPECT_FALSE(bh.IsEnabled());

    EXPECT_EQ(0U, panel0->event_count);
    EXPECT_EQ(0U, panel1->event_count);
    EXPECT_EQ(0U, panel2->event_count);

    // No board is shown => Do not get event.
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled, bh.HandleEvent(event));
    EXPECT_EQ(0U, panel0->event_count);
    EXPECT_EQ(0U, panel1->event_count);
    EXPECT_EQ(0U, panel2->event_count);

    board1->Show(true);
    EXPECT_TRUE(bh.IsEnabled());
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, bh.HandleEvent(event));
    EXPECT_EQ(0U, panel0->event_count);
    EXPECT_EQ(1U, panel1->event_count);
    EXPECT_EQ(0U, panel2->event_count);

    // Show board0 in addition - it should get the event.
    board0->Show(true);
    EXPECT_TRUE(bh.IsEnabled());
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, bh.HandleEvent(event));
    EXPECT_EQ(1U, panel0->event_count);
    EXPECT_EQ(1U, panel1->event_count);
    EXPECT_EQ(0U, panel2->event_count);

    // Hide board0 and board1.
    board0->Show(false);
    board1->Show(false);
    EXPECT_FALSE(bh.IsEnabled());
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled, bh.HandleEvent(event));
    EXPECT_EQ(1U, panel0->event_count);
    EXPECT_EQ(1U, panel1->event_count);
    EXPECT_EQ(0U, panel2->event_count);

    // Show board2 - it should get the event.
    board2->Show(true);
    EXPECT_TRUE(bh.IsEnabled());
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, bh.HandleEvent(event));
    EXPECT_EQ(1U, panel0->event_count);
    EXPECT_EQ(1U, panel1->event_count);
    EXPECT_EQ(1U, panel2->event_count);
}
