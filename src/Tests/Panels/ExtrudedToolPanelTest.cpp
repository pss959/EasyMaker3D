#include "Math/Linear.h"
#include "Models/ExtrudedModel.h"
#include "Panels/ExtrudedToolPanel.h"
#include "Panes/ProfilePane.h"
#include "Panes/TextInputPane.h"
#include "Tests/Panels/ToolPanelTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/Slider2DWidget.h"

/// \ingroup Tests
class ExtrudedToolPanelTest : public ToolPanelTestBase {
  protected:
    ExtrudedToolPanelPtr panel;
    ExtrudedToolPanelTest(bool need_text = false) :
        ToolPanelTestBase(need_text) {
        panel = InitPanel<ExtrudedToolPanel>("ExtrudedToolPanel");
        ObserveChanges(*panel);
    }
};

/// \ingroup Tests
class ExtrudedToolPanelTestWithText : public ExtrudedToolPanelTest {
  protected:
    ExtrudedToolPanelTestWithText() : ExtrudedToolPanelTest(true) {}
};

TEST_F(ExtrudedToolPanelTest, Defaults) {
    EXPECT_EQ(ProfilePane::CreateDefaultProfile(), panel->GetProfile());
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(ExtrudedToolPanelTest, SetProfile) {
    const Profile prof = ExtrudedModel::CreateRegularPolygonProfile(3);
    EXPECT_TRUE(prof.IsValid());
    panel->SetProfile(prof);
    EXPECT_EQ(prof, panel->GetProfile());
}

TEST_F(ExtrudedToolPanelTest, Show) {
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
}

TEST_F(ExtrudedToolPanelTestWithText, Change) {
    panel->SetSize(Vector2f(300, 300));  // Required for text sizing.
    panel->SetIsShown(true);

    // Change the number of sides using text input and the "SetSides" button.
    auto st = SetTextInput("SidesText", "12");
    EXPECT_EQ("12", st->GetText());
    EXPECT_TRUE(st->IsTextValid());
    ClickButtonPane("SetSides");
    EXPECT_EQ(12U, panel->GetProfile().GetPointCount());
    EXPECT_EQ(1U,           GetChangeInfo().count);
    EXPECT_EQ("Profile",    GetChangeInfo().name);
    EXPECT_EQ("kImmediate", GetChangeInfo().type);

    // Drag one of the points.
    auto sw = SG::FindTypedNodeUnderNode<Slider2DWidget>(
        *panel, "MovablePoint_0");
    EXPECT_VECS_CLOSE2(Vector2f(.4f, 0), ToVector2f(sw->GetTranslation()));
    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(.3f, 0, 0), Point3f(0, 0, 0));
    // Drag start/continue/end = 3 changes.
    EXPECT_EQ(4U,         GetChangeInfo().count);
    EXPECT_EQ("Profile",  GetChangeInfo().name);
    EXPECT_EQ("kDragEnd", GetChangeInfo().type);
    EXPECT_VECS_CLOSE2(Vector2f(.1f, 0), ToVector2f(sw->GetTranslation()));

    // Drag with precision turned on.
    panel->SetPrecision(Vector2f(.1f, .1f));
    ToggleCheckboxPane("SnapToPrecision");
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.23f, -.19f, 0));
    // Drag start/continue/end = 3 changes.
    EXPECT_EQ(7U,         GetChangeInfo().count);
    EXPECT_EQ("Profile",  GetChangeInfo().name);
    EXPECT_EQ("kDragEnd", GetChangeInfo().type);
    EXPECT_VECS_CLOSE2(Vector2f(.3f, -.2f), ToVector2f(sw->GetTranslation()));

    // Check for invalid sides text.
    st = SetTextInput("SidesText", "abc");
    EXPECT_FALSE(st->IsTextValid());
}

TEST_F(ExtrudedToolPanelTest, GetGripWidget) {
    EXPECT_TRUE(panel->CanGripHover());
    panel->SetProfile(ExtrudedModel::CreateRegularPolygonProfile(4));

    EXPECT_EQ("MovablePoint_0",
              panel->GetGripWidget(Point2f(.91f, .48f))->GetName());
    EXPECT_EQ("MovablePoint_1",
              panel->GetGripWidget(Point2f(.51f, .88f))->GetName());
    EXPECT_EQ("MovablePoint_2",
              panel->GetGripWidget(Point2f(.02f, .52f))->GetName());
    EXPECT_EQ("MovablePoint_3",
              panel->GetGripWidget(Point2f(.47f, .21f))->GetName());
}
