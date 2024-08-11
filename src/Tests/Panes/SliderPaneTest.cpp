//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Base/Event.h"
#include "Base/VirtualKeyboard.h"
#include "Panes/LabeledSliderPane.h"
#include "Panes/SliderPane.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/General.h"
#include "Widgets/Slider1DWidget.h"

/// \ingroup Tests
class SliderPaneTest : public PaneTestBase {
  protected:
    SliderPanePtr GetSliderPane(const Str &contents = "") {
        return ReadRealPane<SliderPane>("SliderPane", contents);
    }
};

TEST_F(SliderPaneTest, Defaults) {
    auto slider = GetSliderPane();
    EXPECT_EQ(PaneOrientation::kHorizontal, slider->GetOrientation());
    EXPECT_EQ(Vector2f(0, 1),               slider->GetRange());
    EXPECT_EQ(0,                            slider->GetValue());
    EXPECT_EQ(slider.get(),                 slider->GetInteractor());
    EXPECT_TRUE(Util::IsA<Slider1DWidget>(slider->GetActivationWidget()));
    // Cannot focus on a SliderPane.
    EXPECT_NULL(slider->GetFocusBorder());

    // These should fall back to the default IPaneInteractor versions, which do
    // nothing.
    VirtualKeyboardPtr vk(new VirtualKeyboard);
    slider->SetVirtualKeyboard(vk);
    slider->SetFocus(true);
    slider->Activate();
    slider->Deactivate();
    EXPECT_FALSE(slider->IsActive());
    EXPECT_FALSE(slider->HandleEvent(Event()));
}

TEST_F(SliderPaneTest, IsValid) {
    SetParseTypeName("SliderPane");
    TestInvalid("range: 1 1", "Empty or negative range");
    TestInvalid("range: 2 1", "Empty or negative range");
}

TEST_F(SliderPaneTest, Drag) {
    auto slider = GetSliderPane();
    slider->SetRange(Vector2f(0, 10));
    slider->SetLayoutSize(Vector2f(400, 20));

    auto sw = std::dynamic_pointer_cast<Slider1DWidget>(
        slider->GetActivationWidget());
    EXPECT_NOT_NULL(sw);

    size_t   act_count = 0;
    size_t deact_count = 0;
    float      cur_val = -100;
    slider->GetActivation().AddObserver("key", [&](bool is_act){
        if (is_act)
            ++act_count;
        else
            ++deact_count;
    });
    slider->GetValueChanged().AddObserver("key", [&](float v){ cur_val = v; });

    EXPECT_EQ(0U,   act_count);
    EXPECT_EQ(0U, deact_count);
    EXPECT_EQ(-100, cur_val);

    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.5f, 0, 0));
    EXPECT_CLOSE(.555556f, sw->GetValue());
    EXPECT_CLOSE(5.55556f, slider->GetValue());

    EXPECT_EQ(1U,   act_count);
    EXPECT_EQ(1U, deact_count);
    EXPECT_CLOSE(5.55556f, cur_val);
}

TEST_F(SliderPaneTest, DragWithPrecision) {
    auto slider = GetSliderPane();
    slider->SetRange(Vector2f(0, 10));
    slider->SetLayoutSize(Vector2f(400, 20));
    slider->SetPrecision(.1f);

    auto sw = std::dynamic_pointer_cast<Slider1DWidget>(
        slider->GetActivationWidget());
    EXPECT_NOT_NULL(sw);

    size_t   act_count = 0;
    size_t deact_count = 0;
    float      cur_val = -100;
    slider->GetActivation().AddObserver("key", [&](bool is_act){
        if (is_act)
            ++act_count;
        else
            ++deact_count;
    });
    slider->GetValueChanged().AddObserver("key", [&](float v){ cur_val = v; });

    EXPECT_EQ(0U,   act_count);
    EXPECT_EQ(0U, deact_count);
    EXPECT_EQ(-100, cur_val);

    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.5f, 0, 0));
    EXPECT_EQ(.56f,    sw->GetValue());
    EXPECT_CLOSE(5.6f, slider->GetValue());

    EXPECT_EQ(1U,   act_count);
    EXPECT_EQ(1U, deact_count);
    EXPECT_EQ(5.6f, cur_val);
}

TEST_F(SliderPaneTest, DragVertical) {
    auto slider = GetSliderPane(R"(orientation: "kVertical")");
    slider->SetRange(Vector2f(0, 10));
    slider->SetLayoutSize(Vector2f(400, 20));

    auto sw = std::dynamic_pointer_cast<Slider1DWidget>(
        slider->GetActivationWidget());
    EXPECT_NOT_NULL(sw);

    size_t   act_count = 0;
    size_t deact_count = 0;
    float      cur_val = -100;
    slider->GetActivation().AddObserver("key", [&](bool is_act){
        if (is_act)
            ++act_count;
        else
            ++deact_count;
    });
    slider->GetValueChanged().AddObserver("key", [&](float v){ cur_val = v; });

    EXPECT_EQ(0U,   act_count);
    EXPECT_EQ(0U, deact_count);
    EXPECT_EQ(-100, cur_val);

    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.5f, 0, 0));
    EXPECT_CLOSE(.555556f, sw->GetValue());
    EXPECT_CLOSE(5.55556f, slider->GetValue());

    EXPECT_EQ(1U,   act_count);
    EXPECT_EQ(1U, deact_count);
    EXPECT_CLOSE(5.55556f, cur_val);
}

TEST_F(SliderPaneTest, LabeledSliderPane) {
    auto lhsp = ReadRealPane<LabeledSliderPane>("LabeledHSliderPane", "");
    EXPECT_NOT_NULL(lhsp->GetSliderPane());

    SetParseTypeName("LabeledSliderPane");
    TestInvalid("range: 1 1", "Empty or negative range");
    TestInvalid("range: 2 1", "Empty or negative range");
}
