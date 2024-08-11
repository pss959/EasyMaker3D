//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Linear.h"
#include "Models/RevSurfModel.h"
#include "Panels/RevSurfToolPanel.h"
#include "Panes/LabeledSliderPane.h"
#include "Panes/ProfilePane.h"
#include "Panes/SliderPane.h"
#include "Tests/Panels/ToolPanelTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/Slider2DWidget.h"

/// \ingroup Tests
class RevSurfToolPanelTest : public ToolPanelTestBase {
  protected:
    RevSurfToolPanelPtr panel;
    RevSurfToolPanelTest() {
        panel = InitPanel<RevSurfToolPanel>("RevSurfToolPanel");
        ObserveChanges(*panel);
    }
};

TEST_F(RevSurfToolPanelTest, Defaults) {
    EXPECT_EQ(ProfilePane::CreateDefaultProfile(), panel->GetProfile());
    EXPECT_EQ(360, panel->GetSweepAngle().Degrees());
    EXPECT_NULL(panel->GetFocusedPane());
    EXPECT_FALSE(panel->IsCloseable());
    EXPECT_TRUE(panel->IsMovable());
    EXPECT_TRUE(panel->IsResizable());
}

TEST_F(RevSurfToolPanelTest, Set) {
    Profile prof =
        RevSurfModel::CreateProfile(Profile::PointVec{Point2f(.8f, .5f)});
    panel->SetProfile(prof);
    panel->SetSweepAngle(Anglef::FromDegrees(45));
    EXPECT_EQ(prof, panel->GetProfile());
    EXPECT_EQ(45,   panel->GetSweepAngle().Degrees());
}

TEST_F(RevSurfToolPanelTest, Show) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
}

TEST_F(RevSurfToolPanelTest, Change) {
    // Set a reasonable profile.
    panel->SetProfile(
        RevSurfModel::CreateProfile(Profile::PointVec{Point2f(.8f, .5f)}));

    // Drag the sweep angle slider.
    pi.DragSlider("SweepAngleSlider", Vector2f(-.5f, 0));
    EXPECT_EQ(3U,           GetChangeInfo().count);
    EXPECT_EQ("SweepAngle", GetChangeInfo().name);
    EXPECT_EQ("kDragEnd",   GetChangeInfo().type);
    EXPECT_CLOSE(160.5f,    panel->GetSweepAngle().Degrees());

    // Drag the movable point.
    auto sw = SG::FindTypedNodeUnderNode<Slider2DWidget>(
        *panel, "MovablePoint_1");
    EXPECT_VECS_CLOSE2(Vector2f(.3f, 0), ToVector2f(sw->GetTranslation()));
    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(.3f, 0, 0), Point3f(0, 0, 0));
    // Drag start/continue/end = 3 changes.
    EXPECT_EQ(6U,         GetChangeInfo().count);
    EXPECT_EQ("Profile",  GetChangeInfo().name);
    EXPECT_EQ("kDragEnd", GetChangeInfo().type);
    EXPECT_VECS_CLOSE2(Vector2f(0, 0), ToVector2f(sw->GetTranslation()));

    // Drag with precision turned on.
    panel->SetPrecision(Vector2f(.1f, .1f));
    pi.ToggleCheckboxPane("SnapToPrecision");
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.23f, -.19f, 0));
    // Drag start/continue/end = 3 changes.
    EXPECT_EQ(9U,         GetChangeInfo().count);
    EXPECT_EQ("Profile",  GetChangeInfo().name);
    EXPECT_EQ("kDragEnd", GetChangeInfo().type);
    EXPECT_VECS_CLOSE2(Vector2f(.2f, -.2f), ToVector2f(sw->GetTranslation()));
}

TEST_F(RevSurfToolPanelTest, GetGripWidget) {
    EXPECT_TRUE(panel->CanGripHover());

    auto as =
        FindTypedPane<LabeledSliderPane>("SweepAngleSlider")->GetSliderPane();
    auto sw = as->GetActivationWidget();

    EXPECT_EQ(sw,         panel->GetGripWidget(Point2f(.6f, -.4f)));
    EXPECT_EQ("NewPoint", panel->GetGripWidget(Point2f(.5f, .2f))->GetName());
}
