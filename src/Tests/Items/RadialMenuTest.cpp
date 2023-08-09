#include "Items/RadialMenu.h"
#include "Items/RadialMenuInfo.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class RadialMenuTest : public SceneTestBase {};

TEST_F(RadialMenuTest, RadialMenuInfoDefault) {
    auto info = RadialMenuInfo::CreateDefault();
    EXPECT_EQ(RadialMenuInfo::Count::kCount8, info->GetCount());
    EXPECT_EQ(Action::kNone, info->GetButtonAction(0));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(1));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(2));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(3));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(4));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(5));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(6));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(7));
}

TEST_F(RadialMenuTest, RadialMenuInfoActions) {
    auto info = ParseObject<RadialMenuInfo>(
        R"(RadialMenuInfo { count: "kCount2" })");
    EXPECT_EQ(RadialMenuInfo::Count::kCount2, info->GetCount());
    EXPECT_EQ(Action::kNone, info->GetButtonAction(0));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(1));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(2));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(3));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(4));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(5));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(6));
    EXPECT_EQ(Action::kNone, info->GetButtonAction(7));

    info->SetButtonAction(3, Action::kPaste);
    info->SetButtonAction(2, Action::kDelete);
    info->SetButtonAction(1, Action::kCut);
    info->SetButtonAction(0, Action::kCopy);
    EXPECT_EQ(Action::kCopy,   info->GetButtonAction(0));
    EXPECT_EQ(Action::kCut,    info->GetButtonAction(1));
    EXPECT_EQ(Action::kDelete, info->GetButtonAction(2));
    EXPECT_EQ(Action::kPaste,  info->GetButtonAction(3));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(4));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(5));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(6));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(7));

    info->SetCount(RadialMenuInfo::Count::kCount8);
    EXPECT_EQ(RadialMenuInfo::Count::kCount8, info->GetCount());
    EXPECT_EQ(Action::kCopy,   info->GetButtonAction(0));
    EXPECT_EQ(Action::kCut,    info->GetButtonAction(1));
    EXPECT_EQ(Action::kDelete, info->GetButtonAction(2));
    EXPECT_EQ(Action::kPaste,  info->GetButtonAction(3));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(4));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(5));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(6));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(7));

    info->SetCount(RadialMenuInfo::Count::kCount2);
    EXPECT_EQ(RadialMenuInfo::Count::kCount2, info->GetCount());
    EXPECT_EQ(Action::kCopy,   info->GetButtonAction(0));
    EXPECT_EQ(Action::kCut,    info->GetButtonAction(1));
    EXPECT_EQ(Action::kDelete, info->GetButtonAction(2));
    EXPECT_EQ(Action::kPaste,  info->GetButtonAction(3));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(4));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(5));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(6));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(7));

    info->SetCount(RadialMenuInfo::Count::kCount8);
    EXPECT_EQ(RadialMenuInfo::Count::kCount8, info->GetCount());
    EXPECT_EQ(Action::kCopy,   info->GetButtonAction(0));
    EXPECT_EQ(Action::kCut,    info->GetButtonAction(1));
    EXPECT_EQ(Action::kDelete, info->GetButtonAction(2));
    EXPECT_EQ(Action::kPaste,  info->GetButtonAction(3));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(4));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(5));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(6));
    EXPECT_EQ(Action::kNone,   info->GetButtonAction(7));

    auto copy = CreateObject<RadialMenuInfo>();
    copy->CopyFrom(*info);
    EXPECT_EQ(RadialMenuInfo::Count::kCount8, copy->GetCount());
    EXPECT_EQ(Action::kCopy,   copy->GetButtonAction(0));
    EXPECT_EQ(Action::kCut,    copy->GetButtonAction(1));
    EXPECT_EQ(Action::kDelete, copy->GetButtonAction(2));
    EXPECT_EQ(Action::kPaste,  copy->GetButtonAction(3));
    EXPECT_EQ(Action::kNone,   copy->GetButtonAction(4));
    EXPECT_EQ(Action::kNone,   copy->GetButtonAction(5));
    EXPECT_EQ(Action::kNone,   copy->GetButtonAction(6));
    EXPECT_EQ(Action::kNone,   copy->GetButtonAction(7));
}

TEST_F(RadialMenuTest, RadialMenu) {
    const std::string contents = R"(
  children: [
    Node {
      TEMPLATES: [ <"nodes/templates/RadialMenu.emd"> ],
      children:  [ CLONE "T_RadialMenu" "TestMenu"  {} ],
    }
  ]
)";

    auto menu = ReadRealNode<RadialMenu>(contents, "TestMenu");

    size_t clicked_index  = 1000;
    Action clicked_action = Action::kNone;

    auto clicked = [&](size_t index, Action action){
        clicked_index  = index;
        clicked_action = action;
    };
    menu->GetButtonClicked().AddObserver("key", clicked);

    auto info = CreateObject<RadialMenuInfo>();

    info->SetCount(RadialMenuInfo::Count::kCount4);
    info->SetButtonAction(0, Action::kCopy);
    info->SetButtonAction(1, Action::kCut);
    info->SetButtonAction(2, Action::kDelete);
    info->SetButtonAction(3, Action::kPaste);
    menu->UpdateFromInfo(*info);

    // Simulate a click on a real button.
    menu->HighlightButton(Anglef::FromDegrees(135));
    menu->SimulateButtonPress();
    EXPECT_EQ(1U,                 clicked_index);
    EXPECT_ENUM_EQ(Action::kCut,  clicked_action);

    // Change a button action and click again.
    menu->ChangeButtonAction(0, Action::kUndo);
    menu->HighlightButton(Anglef::FromDegrees(45));
    menu->SimulateButtonPress();
    EXPECT_EQ(0U,                 clicked_index);
    EXPECT_ENUM_EQ(Action::kUndo, clicked_action);

    // Bad angle: no change.
    clicked_index  = 1000;
    clicked_action = Action::kNone;
    menu->HighlightButton(Anglef::FromDegrees(-30));
    menu->SimulateButtonPress();
    EXPECT_EQ(1000U,              clicked_index);
    EXPECT_ENUM_EQ(Action::kNone, clicked_action);

    // Disable buttons and click: no change if disabled.
    menu->EnableButtons([](Action act){ return act != Action::kUndo; });
    menu->HighlightButton(Anglef::FromDegrees(45));
    menu->SimulateButtonPress();
    EXPECT_EQ(1000U,              clicked_index);
    EXPECT_ENUM_EQ(Action::kNone, clicked_action);
    menu->HighlightButton(Anglef::FromDegrees(135));
    menu->SimulateButtonPress();
    EXPECT_EQ(1U,                 clicked_index);
    EXPECT_ENUM_EQ(Action::kCut,  clicked_action);
}
