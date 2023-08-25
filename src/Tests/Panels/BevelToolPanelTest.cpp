#include "Panels/BevelToolPanel.h"
#include "Panes/LabeledSliderPane.h"
#include "Panes/ProfilePane.h"
#include "Panes/SliderPane.h"
#include "Tests/Panels/ToolPanelTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/Slider1DWidget.h"

/// \ingroup Tests
class BevelToolPanelTest : public ToolPanelTestBase {
  protected:
    BevelToolPanelPtr panel;
    BevelToolPanelTest() {
        panel = InitPanel<BevelToolPanel>("BevelToolPanel");
        ObserveChanges(*panel);
    }
};

TEST_F(BevelToolPanelTest, Defaults) {
    const Bevel bevel = panel->GetBevel();
    EXPECT_EQ(Profile::CreateFixedProfile(Point2f(0, 1), Point2f(1, 0), 2,
                                          Profile::PointVec()),
              bevel.profile);
    EXPECT_EQ(1,   bevel.scale);
    EXPECT_EQ(120, bevel.max_angle.Degrees());
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(BevelToolPanelTest, SetBevel) {
    Bevel bevel;
    bevel.profile = Profile::CreateFixedProfile(
        Point2f(0, 0), Point2f(0, 1), 3, Profile::PointVec{Point2f(.8f, .5f)});
    bevel.scale = 2.5f;
    bevel.max_angle = Anglef::FromDegrees(135);

    panel->SetBevel(bevel);
    EXPECT_EQ(bevel, panel->GetBevel());
}

TEST_F(BevelToolPanelTest, Show) {
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
}

TEST_F(BevelToolPanelTest, Change) {
    // Drag the scale slider.
    auto sc = FindTypedPane<LabeledSliderPane>("ScaleSlider")->GetSliderPane();
    auto sw =
        std::dynamic_pointer_cast<Slider1DWidget>(sc->GetActivationWidget());
    {
        DragTester dt(sw);
        dt.SetRayDirection(-Vector3f::AxisZ());
        dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.5f, 0, 0));
        // Drag start/continue/end = 3 changes.
        EXPECT_EQ(3U,         GetChangeInfo().count);
        EXPECT_EQ("Scale",    GetChangeInfo().name);
        EXPECT_EQ("kDragEnd", GetChangeInfo().type);
    }

    // Drag the max_angle slider.
    sc = FindTypedPane<LabeledSliderPane>("AngleSlider")->GetSliderPane();
    sw = std::dynamic_pointer_cast<Slider1DWidget>(sc->GetActivationWidget());
    {
        DragTester dt(sw);
        dt.SetRayDirection(-Vector3f::AxisZ());
        dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.5f, 0, 0));
        // Drag start/continue/end = 3 changes.
        EXPECT_EQ(6U,         GetChangeInfo().count);
        EXPECT_EQ("MaxAngle", GetChangeInfo().name);
        EXPECT_EQ("kDragEnd", GetChangeInfo().type);
    }
}

TEST_F(BevelToolPanelTest, GetGripWidget) {
    EXPECT_TRUE(panel->CanGripHover());

    auto get_sw = [&](const Str &name){
        auto sp = FindTypedPane<LabeledSliderPane>(name)->GetSliderPane();
        return sp->GetActivationWidget();
    };

    EXPECT_EQ(get_sw("ScaleSlider"), panel->GetGripWidget(Point2f(.9f, -.4f)));
    EXPECT_EQ(get_sw("AngleSlider"), panel->GetGripWidget(Point2f(.9f, -.1f)));
    EXPECT_EQ("NewPoint", panel->GetGripWidget(Point2f(.5f, .2f))->GetName());
}
