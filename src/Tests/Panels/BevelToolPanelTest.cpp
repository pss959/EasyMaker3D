#include "Panels/BevelToolPanel.h"
#include "Panes/LabeledSliderPane.h"
#include "Panes/ProfilePane.h"
#include "Panes/SliderPane.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/Slider1DWidget.h"

class BevelToolPanelTest : public PanelTestBase {};

TEST_F(BevelToolPanelTest, Defaults) {
    auto panel = ReadRealPanel<BevelToolPanel>("BevelToolPanel");
    const Bevel bevel = panel->GetBevel();
    EXPECT_EQ(Profile::CreateFixedProfile(Point2f(0, 1), Point2f(1, 0), 2,
                                          Profile::PointVec()),
              bevel.profile);
    EXPECT_EQ(1,   bevel.scale);
    EXPECT_EQ(120, bevel.max_angle.Degrees());
}

TEST_F(BevelToolPanelTest, SetBevel) {
    Bevel bevel;
    bevel.profile = Profile::CreateFixedProfile(
        Point2f(0, 0), Point2f(0, 1), 3, Profile::PointVec{Point2f(.8f, .5f)});
    bevel.scale = 2.5f;
    bevel.max_angle = Anglef::FromDegrees(135);

    auto panel = ReadRealPanel<BevelToolPanel>("BevelToolPanel");
    panel->SetBevel(bevel);
    EXPECT_EQ(bevel, panel->GetBevel());
}

TEST_F(BevelToolPanelTest, Show) {
    auto panel = ReadRealPanel<BevelToolPanel>("BevelToolPanel");
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
}

TEST_F(BevelToolPanelTest, Change) {
    auto panel = ReadRealPanel<BevelToolPanel>("BevelToolPanel");

    size_t                     change_count = 0;
    Str                        change_name;
    ToolPanel::InteractionType change_type;

    panel->GetInteraction().AddObserver(
        "key", [&](const Str &name, ToolPanel::InteractionType type){
            ++change_count;
            change_name = name;
            change_type = type;
        });

    // Drag the scale slider.
    auto sc =
        FindPane<LabeledSliderPane>(*panel, "ScaleSlider")->GetSliderPane();
    auto sw =
        std::dynamic_pointer_cast<Slider1DWidget>(sc->GetActivationWidget());
    {
        DragTester dt(sw);
        dt.SetRayDirection(-Vector3f::AxisZ());
        dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.5f, 0, 0));
        // Drag start/continue/end = 3 changes.
        EXPECT_EQ(3U,                                   change_count);
        EXPECT_EQ("Scale",                              change_name);
        EXPECT_EQ(ToolPanel::InteractionType::kDragEnd, change_type);
    }

    // Drag the max_angle slider.
    sc = FindPane<LabeledSliderPane>(*panel, "AngleSlider")->GetSliderPane();
    sw = std::dynamic_pointer_cast<Slider1DWidget>(sc->GetActivationWidget());
    {
        DragTester dt(sw);
        dt.SetRayDirection(-Vector3f::AxisZ());
        dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.5f, 0, 0));
        // Drag start/continue/end = 3 changes.
        EXPECT_EQ(6U,                                   change_count);
        EXPECT_EQ("MaxAngle",                           change_name);
        EXPECT_EQ(ToolPanel::InteractionType::kDragEnd, change_type);
    }
}

TEST_F(BevelToolPanelTest, GetGripWidget) {
    auto panel = ReadRealPanel<BevelToolPanel>("BevelToolPanel");
    EXPECT_TRUE(panel->CanGripHover());

    auto get_sw = [&](const Str &name){
        auto sp = FindPane<LabeledSliderPane>(*panel, name)->GetSliderPane();
        return sp->GetActivationWidget();
    };

    EXPECT_EQ(get_sw("ScaleSlider"), panel->GetGripWidget(Point2f(.9f, -.4f)));
    EXPECT_EQ(get_sw("AngleSlider"), panel->GetGripWidget(Point2f(.9f, -.1f)));
    EXPECT_EQ("NewPoint", panel->GetGripWidget(Point2f(.5f, .2f))->GetName());
}
