#include "Panels/TaperToolPanel.h"
#include "Panes/ProfilePane.h"
#include "Tests/Panels/ToolPanelTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/Slider2DWidget.h"

/// \ingroup Tests
class TaperToolPanelTest : public ToolPanelTestBase {
  protected:
    TaperToolPanelPtr panel;
    TaperToolPanelTest() {
        panel = InitPanel<TaperToolPanel>("TaperToolPanel");
        ObserveChanges(*panel);
    }
};

TEST_F(TaperToolPanelTest, Defaults) {
    EXPECT_ENUM_EQ(Dim::kY, panel->GetTaper().axis);
    EXPECT_EQ(ProfilePane::CreateDefaultProfile(), panel->GetTaper().profile);
    EXPECT_NULL(panel->GetFocusedPane());
    EXPECT_FALSE(panel->IsCloseable());
}

TEST_F(TaperToolPanelTest, SetTaper) {
    Taper taper;
    taper.axis    = Dim::kZ;
    taper.profile = Profile(Profile::Type::kOpen, 2,
                            Profile::PointVec{Point2f(1, 1), Point2f(.5f, 0)});
    panel->SetTaper(taper);
    EXPECT_EQ(taper, panel->GetTaper());
}

TEST_F(TaperToolPanelTest, Show) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
}

TEST_F(TaperToolPanelTest, Change) {
    // Set a reasonable Taper profile.
    Taper taper;
    taper.profile = Profile(Profile::Type::kOpen, 2,
                            Profile::PointVec{Point2f(1, 1), Point2f(1, 0)});
    panel->SetTaper(taper);

    // Drag the top movable point.
    auto sw = SG::FindTypedNodeUnderNode<Slider2DWidget>(
        *panel, "MovablePoint_0");
    EXPECT_VECS_CLOSE2(Vector2f(.5f, .5f), ToVector2f(sw->GetTranslation()));
    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(.3f, 0, 0), Point3f(0, 0, 0));
    // Drag start/continue/end = 3 changes.
    EXPECT_EQ(3U,         GetChangeInfo().count);
    EXPECT_EQ("Profile",  GetChangeInfo().name);
    EXPECT_EQ("kDragEnd", GetChangeInfo().type);
    EXPECT_VECS_CLOSE2(Vector2f(.2f, .5f), ToVector2f(sw->GetTranslation()));

    // Change the dimension.
    ActivateRadioButtonPane("Y");  // User "Y" axis == app "Z" axis.
    EXPECT_ENUM_EQ(Dim::kZ, panel->GetTaper().axis);
    EXPECT_EQ(4U,           GetChangeInfo().count);
    EXPECT_EQ("Axis",       GetChangeInfo().name);
    EXPECT_EQ("kImmediate", GetChangeInfo().type);
}

TEST_F(TaperToolPanelTest, GetGripWidget) {
    EXPECT_TRUE(panel->CanGripHover());

    // Set a reasonable Taper profile.
    Taper taper;
    taper.profile = Profile(Profile::Type::kOpen, 2,
                            Profile::PointVec{Point2f(1, 1), Point2f(1, 0)});
    panel->SetTaper(taper);

    EXPECT_EQ("MovablePoint_0",
              panel->GetGripWidget(Point2f(.9f, .9f))->GetName());
    EXPECT_EQ("MovablePoint_1",
              panel->GetGripWidget(Point2f(.9f, .2f))->GetName());
    EXPECT_EQ("NewPoint",
              panel->GetGripWidget(Point2f(.5f, .5f))->GetName());
}
