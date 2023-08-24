#include "Math/Linear.h"
#include "Models/ExtrudedModel.h"
#include "Panels/ExtrudedToolPanel.h"
#include "Panes/ProfilePane.h"
#include "Panes/TextInputPane.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"
#include "Tests/UnitTestTypeChanger.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/Slider2DWidget.h"

class ExtrudedToolPanelTest : public PanelTestBase {};

TEST_F(ExtrudedToolPanelTest, Defaults) {
    auto panel = ReadRealPanel<ExtrudedToolPanel>("ExtrudedToolPanel");
    EXPECT_EQ(Profile::CreateFixedProfile(Point2f(0, 1), Point2f(1, 0), 2,
                                          Profile::PointVec()),
              panel->GetProfile());
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(ExtrudedToolPanelTest, SetProfile) {
    auto panel = ReadRealPanel<ExtrudedToolPanel>("ExtrudedToolPanel");
    const Profile prof = ExtrudedModel::CreateRegularPolygonProfile(3);
    EXPECT_TRUE(prof.IsValid());
    panel->SetProfile(prof);
    EXPECT_EQ(prof, panel->GetProfile());
}

TEST_F(ExtrudedToolPanelTest, Show) {
    auto panel = ReadRealPanel<ExtrudedToolPanel>("ExtrudedToolPanel");
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
}

TEST_F(ExtrudedToolPanelTest, Change) {
    // Override this setting; the TextInputPane needs to build font images.
    UnitTestTypeChanger uttc(Util::AppType::kInteractive);

    auto panel = ReadRealPanel<ExtrudedToolPanel>("ExtrudedToolPanel");

    size_t                     change_count = 0;
    Str                        change_name;
    ToolPanel::InteractionType change_type;

    panel->GetInteraction().AddObserver(
        "key", [&](const Str &name, ToolPanel::InteractionType type){
            ++change_count;
            change_name = name;
            change_type = type;
        });

    panel->SetSize(Vector2f(300, 300));  // Required for text sizing.
    panel->SetIsShown(true);

    // Change the number of sides using text input and the "SetSides" button.
    auto st = FindTypedPane<TextInputPane>(*panel, "SidesText");
    st->GetInteractor()->Activate();
    st->SetInitialText("12");
    EXPECT_EQ("12", st->GetText());
    EXPECT_TRUE(st->IsTextValid());
    st->GetInteractor()->Deactivate();
    ClickButtonPane(*panel, "SetSides");
    EXPECT_EQ(12U, panel->GetProfile().GetPointCount());
    EXPECT_EQ(1U,                                     change_count);
    EXPECT_EQ("Profile",                              change_name);
    EXPECT_EQ(ToolPanel::InteractionType::kImmediate, change_type);

    // Drag one of the points.
    auto sw = SG::FindTypedNodeUnderNode<Slider2DWidget>(
        *panel, "MovablePoint_0");
    EXPECT_VECS_CLOSE2(Vector2f(.4f, 0), ToVector2f(sw->GetTranslation()));
    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(.3f, 0, 0), Point3f(0, 0, 0));
    // Drag start/continue/end = 3 changes.
    EXPECT_EQ(4U,                                   change_count);
    EXPECT_EQ("Profile",                            change_name);
    EXPECT_EQ(ToolPanel::InteractionType::kDragEnd, change_type);
    EXPECT_VECS_CLOSE2(Vector2f(.1f, 0), ToVector2f(sw->GetTranslation()));

    // Drag with precision turned on.
    panel->SetPrecision(Vector2f(.1f, .1f));
    ToggleCheckboxPane(*panel, "SnapToPrecision");
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.23f, -.19f, 0));
    // Drag start/continue/end = 3 changes.
    EXPECT_EQ(7U,                                   change_count);
    EXPECT_EQ("Profile",                            change_name);
    EXPECT_EQ(ToolPanel::InteractionType::kDragEnd, change_type);
    EXPECT_VECS_CLOSE2(Vector2f(.3f, -.2f), ToVector2f(sw->GetTranslation()));

    // Check for invalid sides text.
    st->GetInteractor()->Activate();
    st->SetInitialText("abc");
    st->GetInteractor()->Deactivate();
    EXPECT_FALSE(st->IsTextValid());
}

TEST_F(ExtrudedToolPanelTest, GetGripWidget) {
    auto panel = ReadRealPanel<ExtrudedToolPanel>("ExtrudedToolPanel");
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
