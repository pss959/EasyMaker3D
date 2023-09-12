#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Panels/Board.h"
#include "Panels/HelpPanel.h"
#include "Panes/ButtonPane.h"
#include "Panes/TextPane.h"
#include "SG/Search.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider2DWidget.h"

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

TEST_F(BoardTest, Panels) {
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

TEST_F(BoardTest, PanelScale) {
    InitBoard();

    // Set up for touch to make sure it is handle.
    board->SetUpForTouch(Point3f(0, 0, 10));

    auto canvas = SG::FindNodeUnderNode(*board, "Canvas");

    // Need a resizable Panel for this.
    auto panel = GetContext().board_agent->GetPanel("InfoPanel");
    board->SetPanel(panel, nullptr);

    const auto canvas_scale = ToVector2f(canvas->GetScale());

    // Change the scale and set the Panel again to get the Board to update.
    board->SetPanelScale(2 * TK::kPanelToWorldScale);
    board->PopPanel("Done");
    board->SetPanel(panel, nullptr);
    EXPECT_EQ(2 * canvas_scale, ToVector2f(canvas->GetScale()));

    // Cause the Panel width to change and make sure the Board updates.
    const auto canvas_size = ToVector2f(canvas->GetScaledBounds().GetSize());
    auto title = panel->GetPane()->FindTypedSubPane<TextPane>("Title");
    title->SetText("Some text that causes a width change");
    board->UpdateForRenderPass("");  // Causes size change to take effect.
    EXPECT_LT(canvas_size[0], canvas->GetScaledBounds().GetSize()[0]);
    EXPECT_EQ(canvas_size[1], canvas->GetScaledBounds().GetSize()[1]);
}

TEST_F(BoardTest, Show) {
    InitBoard();

    auto panel = GetContext().board_agent->GetPanel("HelpPanel");
    board->SetPanel(panel, nullptr);

    EXPECT_FALSE(board->IsShown());

    board->Show(true);
    EXPECT_TRUE(board->IsShown());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    board->Show(false);
    EXPECT_FALSE(board->IsShown());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
}

TEST_F(BoardTest, Transform) {
    InitBoard();

    EXPECT_EQ(Rotationf::Identity(), board->GetRotation());
    EXPECT_EQ(Vector3f::Zero(),     board->GetTranslation());

    // Look down on the Board; it should orient to face up.
    board->SetOrientation(-Vector3f::AxisY());
    EXPECT_VECS_CLOSE(Vector3f::AxisY(),
                      board->GetRotation() * Vector3f::AxisZ());
    EXPECT_EQ(Vector3f::Zero(), board->GetTranslation());

    board->SetPosition(Point3f(10, 20, 30));
    EXPECT_VECS_CLOSE(Vector3f::AxisY(),
                      board->GetRotation() * Vector3f::AxisZ());
    EXPECT_EQ(Vector3f(10, 20, 30), board->GetTranslation());
}

TEST_F(BoardTest, DragMove) {
    InitBoard();

    EXPECT_EQ(Vector3f::Zero(), board->GetTranslation());

    auto xy = SG::FindTypedNodeUnderNode<Slider2DWidget>(*board,
                                                         "XYMoveSlider");
    auto xz = SG::FindTypedNodeUnderNode<Slider2DWidget>(*board,
                                                         "XZMoveSlider");

    // Drag in X and Y.
    DragTester dtxy(xy);
    dtxy.ApplyMouseDrag(Point3f(0, 0, 2), Point3f(4, 4, 2));
    EXPECT_EQ(Vector3f(4, 4, 0), board->GetTranslation());

    // Drag with the XZ slider.
    DragTester dtxz(xz);
    dtxz.ApplyMouseDrag(Point3f(0, 2, 0), Point3f(-4, 2, 0));
    EXPECT_EQ(Vector3f(0, 4, 0), board->GetTranslation());
}

TEST_F(BoardTest, DragSize) {
    InitBoard();

    // Need a resizable Panel for this.
    auto panel = GetContext().board_agent->GetPanel("InfoPanel");
    panel->SetSize(Vector2f(600, 600));
    board->SetPanel(panel, nullptr);

    // Use the top-right handle of the size slider.
    auto sz = SG::FindTypedNodeUnderNode<Slider2DWidget>(*board, "SizeSlider");
    auto pt = SG::FindNodeUnderNode(*sz, "TopRight");
    DragTester dttr(sz, pt);
    dttr.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(4, 4, 0));
    EXPECT_EQ(Vector2f(750, 750), panel->GetSize());

    // Repeat with bottom-right handle.
    pt = SG::FindNodeUnderNode(*sz, "BottomRight");
    DragTester dtbl(sz, pt);
    dtbl.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(4, -4, 0));
    EXPECT_EQ(Vector2f(900, 900), panel->GetSize());
}

TEST_F(BoardTest, Grip) {
    InitBoard();

    board->SetPath(SG::NodePath(board));

    // Use a resizable Panel to test size handles as well as move handles, and
    // use a Panel that accepts Grips to test delegation to the Panel.
    auto panel = GetContext().board_agent->GetPanel("ExtrudedToolPanel");
    panel->SetSize(Vector2f(600, 600));
    board->SetPanel(panel, nullptr);

    auto xy = SG::FindTypedNodeUnderNode<Slider2DWidget>(*board,
                                                         "XYMoveSlider");
    auto xz = SG::FindTypedNodeUnderNode<Slider2DWidget>(*board,
                                                         "XZMoveSlider");
    auto sz = SG::FindTypedNodeUnderNode<Slider2DWidget>(*board, "SizeSlider");

    EXPECT_NULL(board->GetGrippableNode());  // Not shown.
    board->Show(true);
    EXPECT_EQ(board.get(), board->GetGrippableNode());

    Grippable::GripInfo info;
    info.event.device = Event::Device::kRightController;

    // Board requires only the guide direction to be set.

    // Pointing toward Panel means ask the Panel.
    info.guide_direction.Set(0, 0, -1);
    board->UpdateGripInfo(info);
    EXPECT_NOT_NULL(info.widget);
    EXPECT_EQ("NewPoint", info.widget->GetName());

    // Guide direction pointing left should grab the right move handle.
    info.guide_direction.Set(-1, 0, 0);
    board->UpdateGripInfo(info);
    EXPECT_NOT_NULL(info.widget);
    EXPECT_EQ("XYMoveSlider",       info.widget->GetName());
    EXPECT_EQ(Point3f(18.5f, 0, 0), info.target_point);

    // Guide direction pointing right should grab the left move handle.
    info.guide_direction.Set(1, 0, 0);
    board->UpdateGripInfo(info);
    EXPECT_NOT_NULL(info.widget);
    EXPECT_EQ("XYMoveSlider",        info.widget->GetName());
    EXPECT_EQ(Point3f(-18.5f, 0, 0), info.target_point);

    // Guide direction pointing down should grab the top move handle.
    info.guide_direction.Set(0, -1, 0);
    board->UpdateGripInfo(info);
    EXPECT_NOT_NULL(info.widget);
    EXPECT_EQ("XYMoveSlider",       info.widget->GetName());
    EXPECT_EQ(Point3f(0, 18.5f, 0), info.target_point);

    // Guide direction pointing up should grab the bottom move handle.
    info.guide_direction.Set(0, 1, 0);
    board->UpdateGripInfo(info);
    EXPECT_NOT_NULL(info.widget);
    EXPECT_EQ("XYMoveSlider",        info.widget->GetName());
    EXPECT_EQ(Point3f(0, -18.5f, 0), info.target_point);

    // Angled should grab one of the size handles.
    info.guide_direction = ion::math::Normalized(Vector3f(-1, -1, 0));
    board->UpdateGripInfo(info);
    EXPECT_NOT_NULL(info.widget);
    EXPECT_EQ("SizeSlider",            info.widget->GetName());
    EXPECT_EQ(Point3f(18.5, 18.5f, 0), info.target_point);
    info.guide_direction = ion::math::Normalized(Vector3f(1, 1, 0));
    board->UpdateGripInfo(info);
    EXPECT_NOT_NULL(info.widget);
    EXPECT_EQ("SizeSlider",              info.widget->GetName());
    EXPECT_EQ(Point3f(-18.5, -18.5f, 0), info.target_point);

    // Hover the right move handle and drag it.
    info.guide_direction.Set(-1, 0, 0);
    board->UpdateGripInfo(info);
    board->ActivateGrip(Hand::kRight, true);
    {
        DragTester dt(xy);
        dt.ApplyGripDrag(Point3f(0, 0, 2), Point3f(1, 1, 2));
    }
    EXPECT_EQ(Vector3f(40, 40, 0), board->GetTranslation());
    board->ActivateGrip(Hand::kRight, false);

    // Hover the top-right size handle and drag it.
    auto pt = SG::FindNodeUnderNode(*sz, "TopRight");
    info.guide_direction = ion::math::Normalized(Vector3f(-1, -1, 0));
    board->UpdateGripInfo(info);
    board->ActivateGrip(Hand::kRight, true);
    {
        DragTester dt(sz, pt);
        dt.ApplyGripDrag(Point3f(0, 0, 0), Point3f(1, 1, 0));
    }
    EXPECT_EQ(Vector2f(1950, 1950), panel->GetSize());
    board->ActivateGrip(Hand::kRight, false);

    // Repeat with the bottom-right size handle.
    pt = SG::FindNodeUnderNode(*sz, "BottomRight");
    info.guide_direction = ion::math::Normalized(Vector3f(-1, 1, 0));
    board->UpdateGripInfo(info);
    board->ActivateGrip(Hand::kRight, true);
    {
        DragTester dt(sz, pt);
        dt.ApplyGripDrag(Point3f(0, 0, 0), Point3f(1, -1, 0));
    }
    EXPECT_EQ(Vector2f(3300, 3300), panel->GetSize());
    board->ActivateGrip(Hand::kRight, false);

    // Use the left controller to hover the XZ slider and drag it. Use a
    // direction that is just angled enough to not delegate to the Panel.
    const auto rot = Rotationf::FromAxisAndAngle(
        Vector3f(1, 0, 0), TK::kMaxGripHoverDirAngle + Anglef::FromDegrees(1));
    info.guide_direction = rot * -Vector3f::AxisZ();
    info.event.device = Event::Device::kLeftController;
    board->UpdateGripInfo(info);
    board->ActivateGrip(Hand::kLeft, true);
    {
        // The XZ slider is rotated, so translate in Y to translate in Z.
        DragTester dt(xz);
        dt.ApplyGripDrag(Point3f(0, 0, 0), Point3f(0, 2, 0));
    }
    EXPECT_EQ(Vector3f(40, 40, 40), board->GetTranslation());
    board->ActivateGrip(Hand::kLeft, false);
}

TEST_F(BoardTest, Touch) {
    InitBoard();

    board->SetPath(SG::NodePath(board));
    auto panel = GetContext().board_agent->GetPanel("InfoPanel");
    board->SetPanel(panel, nullptr);
    board->Show(true);

    // Note: touch points are in world coordinates.

    board->SetUpForTouch(Point3f(0, 0, 10));
    EXPECT_EQ(Rotationf::Identity(), board->GetRotation());
    EXPECT_EQ(Vector3f(0, 0, 9.4f),  board->GetTranslation());

    EXPECT_NULL(board->GetTouchedWidget(Point3f(-10, 0, 9.4f), .1f));

    // This should touch the "Done" button.
    auto w = board->GetTouchedWidget(Point3f(.16f, -.1f, 9.4f), .1f);
    EXPECT_NOT_NULL(w);
    auto but = panel->GetPane()->FindTypedSubPane<ButtonPane>("Done");
    EXPECT_EQ(&but->GetButton(), w.get());

    // These should have no effect while the Board is set up for touch.
    const auto rot = board->GetRotation();
    const auto tr  = board->GetTranslation();
    board->SetOrientation(Vector3f(0, 1, 0));
    board->SetPosition(Point3f(1, 2, 3));
    EXPECT_EQ(rot, board->GetRotation());
    EXPECT_EQ(tr,  board->GetTranslation());

    // Disable touch for the Board.
    board->SetUpForTouch(Point3f(0, 0, 0));  // Turns it off.
    EXPECT_EQ(Rotationf::Identity(), board->GetRotation());
    EXPECT_EQ(Vector3f(0, 0, 0),     board->GetTranslation());

    // These should now work.
    board->SetOrientation(-Vector3f::AxisY());
    EXPECT_VECS_CLOSE(Vector3f::AxisY(),
                      board->GetRotation() * Vector3f::AxisZ());
    EXPECT_EQ(Vector3f::Zero(), board->GetTranslation());

    board->SetPosition(Point3f(10, 20, 30));
    EXPECT_VECS_CLOSE(Vector3f::AxisY(),
                      board->GetRotation() * Vector3f::AxisZ());
    EXPECT_EQ(Vector3f(10, 20, 30), board->GetTranslation());
}
