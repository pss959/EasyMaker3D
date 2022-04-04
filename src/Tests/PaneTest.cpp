#include "Defaults.h"
#include "Panes/BoxPane.h"
#include "Panes/TextPane.h"
#include "SG/Search.h"
#include "SceneTestBase.h"
#include "Testing.h"

class PaneTest : public SceneTestBase {
  protected:
    SG::ScenePtr ReadPaneScene() {
        const std::string input = ReadDataFile("Panes.mvn");
        return ReadScene(input);
    }
    PanePtr FindPaneOrAssert(ContainerPane &ctr, const std::string &name) {
        PanePtr pane = ctr.FindPane(name);
        ASSERT(pane);
        return pane;
    }
};

TEST_F(PaneTest, TextPane) {
    SG::ScenePtr scene = ReadPaneScene();
    TextPanePtr text = SG::FindTypedNodeInScene<TextPane>(*scene, "Text");
    text->SetText("ABC");
    text->SetFontSize(22);
    EXPECT_EQ(Vector2f(44.6875f, 22.f), text->GetBaseSize());

    text->SetFontSize(44);
    EXPECT_EQ(Vector2f(89.375, 44.f), text->GetBaseSize());
}

TEST_F(PaneTest, VBoxPane) {
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

TEST_F(PaneTest, HBoxPane) {
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

// XXXX Make sure to test all Pane functions that issue PaneChanged() to make
// XXXX sure sizes update.
