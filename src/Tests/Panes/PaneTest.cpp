#include "Panes/BoxPane.h"
#include "Panes/DropdownPane.h"
#include "Panes/ScrollingPane.h"
#include "Panes/SwitcherPane.h"
#include "Panes/TextPane.h"
#include "SG/Search.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/UnitTestTypeChanger.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class PaneTest : public SceneTestBase {
  protected:
    SG::ScenePtr ReadPaneScene() {
        const std::string input = ReadDataFile("Panes.emd");
        return ReadScene(input);
    }
    PanePtr FindPaneOrAssert(ContainerPane &ctr, const std::string &name) {
        PanePtr pane = ctr.FindPane(name);
        ASSERT(pane);
        return pane;
    }
};

TEST_F(PaneTest, Text) {
    // Override this setting for this test; need to build font images.
    UnitTestTypeChanger uttc(Util::AppType::kInteractive);

    SG::ScenePtr scene = ReadPaneScene();
    TextPanePtr text = SG::FindTypedNodeInScene<TextPane>(*scene, "Text");
    text->SetText("ABC");
    text->SetFontSize(22);
    EXPECT_EQ(Vector2f(44.6875f, 22.f), text->GetBaseSize());

    text->SetFontSize(44);
    EXPECT_EQ(Vector2f(89.375, 44.f), text->GetBaseSize());

    text->SetLayoutSize(Vector2f(100, 60));
    EXPECT_EQ(Vector2f(100, 60), text->GetLayoutSize());
}

TEST_F(PaneTest, VBox) {
    SG::ScenePtr scene = ReadPaneScene();
    BoxPanePtr vbox = SG::FindTypedNodeInScene<BoxPane>(*scene, "VBox");
    PanePtr spacer1 = FindPaneOrAssert(*vbox, "Spacer1");
    PanePtr spacer2 = FindPaneOrAssert(*vbox, "Spacer2");
    PanePtr spacer3 = FindPaneOrAssert(*vbox, "Spacer3");

    // These values are specified in the input.
    EXPECT_EQ(4,  vbox->GetPadding());
    EXPECT_EQ(10, vbox->GetSpacing());
    EXPECT_EQ(Vector2f(8, 15), spacer1->GetMinSize());
    EXPECT_EQ(Vector2f(1,  1), spacer2->GetMinSize());
    EXPECT_EQ(Vector2f(1,  1), spacer3->GetMinSize());
    EXPECT_EQ(Vector2f(8, 15), spacer1->GetBaseSize());
    EXPECT_EQ(Vector2f(1,  1), spacer2->GetBaseSize());
    EXPECT_EQ(Vector2f(1,  1), spacer3->GetBaseSize());

    const Vector2f vbs((2 * 4) + 8,
                       (2 * 4) + (2 * 10) + (1 + 15 + 1));
    EXPECT_EQ(vbs, vbox->GetBaseSize());
    EXPECT_EQ(Vector2f::Zero(), vbox->GetLayoutSize());  // Never layed out.
    vbox->SetLayoutSize(vbs);
    EXPECT_EQ(vbs, vbox->GetBaseSize());
    EXPECT_EQ(vbs, vbox->GetLayoutSize());
    EXPECT_EQ(Vector2f(8, 15), spacer1->GetLayoutSize());
    EXPECT_EQ(Vector2f(8,  1), spacer2->GetLayoutSize());
    EXPECT_EQ(Vector2f(1,  1), spacer3->GetLayoutSize());

    // Increase the layout size of the box. Only Spacer2 should change size.
    const Vector2f vls = vbs + Vector2f(100, 200);
    vbox->SetLayoutSize(vls);
    EXPECT_EQ(vbs, vbox->GetBaseSize());
    EXPECT_EQ(vls, vbox->GetLayoutSize());
    EXPECT_EQ(Vector2f(8,    15), spacer1->GetLayoutSize());
    EXPECT_EQ(Vector2f(108, 201), spacer2->GetLayoutSize());
    EXPECT_EQ(Vector2f(1,     1), spacer3->GetLayoutSize());
}

TEST_F(PaneTest, HBox) {
    SG::ScenePtr scene = ReadPaneScene();
    BoxPanePtr hbox = SG::FindTypedNodeInScene<BoxPane>(*scene, "HBox");
    PanePtr spacer1 = FindPaneOrAssert(*hbox, "Spacer1");
    PanePtr spacer2 = FindPaneOrAssert(*hbox, "Spacer2");
    PanePtr spacer3 = FindPaneOrAssert(*hbox, "Spacer3");

    // These values are specified in the input.
    EXPECT_EQ(4,  hbox->GetPadding());
    EXPECT_EQ(10, hbox->GetSpacing());
    EXPECT_EQ(Vector2f(15, 8), spacer1->GetMinSize());
    EXPECT_EQ(Vector2f(1,  1), spacer2->GetMinSize());
    EXPECT_EQ(Vector2f(1,  1), spacer3->GetMinSize());
    EXPECT_EQ(Vector2f(15, 8), spacer1->GetBaseSize());
    EXPECT_EQ(Vector2f(1,  1), spacer2->GetBaseSize());
    EXPECT_EQ(Vector2f(1,  1), spacer3->GetBaseSize());

    const Vector2f hbs((2 * 4) + (2 * 10) + (1 + 15 + 1),
                       (2 * 4) + 8);
    EXPECT_EQ(hbs, hbox->GetBaseSize());
    EXPECT_EQ(Vector2f::Zero(), hbox->GetLayoutSize());  // Never layed out.
    hbox->SetLayoutSize(hbs);
    EXPECT_EQ(hbs, hbox->GetBaseSize());
    EXPECT_EQ(hbs, hbox->GetLayoutSize());
    EXPECT_EQ(Vector2f(15, 8), spacer1->GetLayoutSize());
    EXPECT_EQ(Vector2f(1,  8), spacer2->GetLayoutSize());
    EXPECT_EQ(Vector2f(1,  1), spacer3->GetLayoutSize());

    // Increase the layout size of the box. Only Spacer2 should change size.
    const Vector2f hls = hbs + Vector2f(200, 100);
    hbox->SetLayoutSize(hls);
    EXPECT_EQ(hbs, hbox->GetBaseSize());
    EXPECT_EQ(hls, hbox->GetLayoutSize());
    EXPECT_EQ(Vector2f(15,    8), spacer1->GetLayoutSize());
    EXPECT_EQ(Vector2f(201, 108), spacer2->GetLayoutSize());
    EXPECT_EQ(Vector2f(1,     1), spacer3->GetLayoutSize());
}

TEST_F(PaneTest, Dropdown) {
    // Override this setting for this test; need to build font images.
    UnitTestTypeChanger uttc(Util::AppType::kInteractive);

    SG::ScenePtr scene = ReadPaneScene();
    auto dd = SG::FindTypedNodeInScene<DropdownPane>(*scene, "Dropdown");

    std::vector<std::string> choices{ "Abcd", "Efgh Ijklmn", "Op Qrstu" };
    dd->SetChoices(choices, 2);
    dd->SetLayoutSize(Vector2f(100, 20));

    EXPECT_EQ(2, dd->GetChoiceIndex());
    EXPECT_EQ("Op Qrstu", dd->GetChoice());

    // The base size of the DropdownPane is the size of the largest choice.
    EXPECT_EQ(Vector2f(88.425f, 20), dd->GetBaseSize());

    // Changing the choice should not affect the base size.
    dd->SetChoice(0);
    EXPECT_EQ(0, dd->GetChoiceIndex());
    EXPECT_EQ("Abcd", dd->GetChoice());
    EXPECT_EQ(Vector2f(88.425f, 20), dd->GetBaseSize());

    // Each choice button in the dropdown should have a non-zero layout size.
    for (const auto &but: dd->GetMenuPane().GetContentsPane()->GetPanes()) {
        const Vector2f &ls = but->GetLayoutSize();
        EXPECT_NE(0, ls[0]);
        EXPECT_NE(0, ls[1]);
    }

    dd->SetChoice(1);
    EXPECT_EQ(1, dd->GetChoiceIndex());
    EXPECT_EQ("Efgh Ijklmn", dd->GetChoice());

    dd->SetChoiceFromString("Op Qrstu");
    EXPECT_EQ(2, dd->GetChoiceIndex());
    EXPECT_EQ("Op Qrstu", dd->GetChoice());
}

TEST_F(PaneTest, Switcher) {
    SG::ScenePtr scene = ReadPaneScene();

    bool changed = false;

    // SwitcherPane with 3 10x10 buttons.
    auto sw = SG::FindTypedNodeInScene<SwitcherPane>(*scene, "Switcher");
    EXPECT_EQ(3U, sw->GetPanes().size());
    sw->GetContentsChanged().AddObserver("TEST", [&changed]{ changed = true; });
    const Vector2f v10x10(10, 10);

    sw->SetLayoutSize(v10x10);
    EXPECT_EQ(v10x10, sw->GetBaseSize());
    EXPECT_EQ(v10x10, sw->GetLayoutSize());

    EXPECT_EQ(-1, sw->GetIndex());
    EXPECT_FALSE(changed);
    EXPECT_EQ(v10x10, sw->GetBaseSize());
    EXPECT_FALSE(sw->GetPanes()[0]->IsEnabled());
    EXPECT_FALSE(sw->GetPanes()[1]->IsEnabled());
    EXPECT_FALSE(sw->GetPanes()[2]->IsEnabled());

    sw->SetIndex(1);
    EXPECT_TRUE(changed);
    EXPECT_EQ(v10x10, sw->GetBaseSize());
    EXPECT_FALSE(sw->GetPanes()[0]->IsEnabled());
    EXPECT_TRUE(sw->GetPanes()[1]->IsEnabled());
    EXPECT_FALSE(sw->GetPanes()[2]->IsEnabled());

    sw->SetLayoutSize(v10x10);
    EXPECT_EQ(v10x10, sw->GetPanes()[1]->GetBaseSize());
    EXPECT_EQ(v10x10, sw->GetPanes()[1]->GetLayoutSize());
    EXPECT_EQ(Vector3f(0, 0, TK::kPaneZOffset),
              sw->GetPanes()[1]->GetTranslation());
}

// TODO Test all Pane functions that issue PaneChanged() to make sure sizes
// update.
