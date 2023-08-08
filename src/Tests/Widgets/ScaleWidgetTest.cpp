#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/String.h"
#include "Util/Tuning.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/ScaleWidget.h"

class ScaleWidgetTest : public SceneTestBase {
  protected:
    ScaleWidgetPtr GetScaleWidget(bool use_modified_mode = false);
};

ScaleWidgetPtr ScaleWidgetTest::GetScaleWidget(bool use_modified_mode) {
    const std::string input = R"(
ScaleWidget "TestScaleWidget" {
  use_modified_mode: <UMM>,
  children: [
    Slider1DWidget "MinSlider" {},
    Slider1DWidget "MaxSlider" {},
    Node "Stick" { shapes: [ Box {} ] },
  ],
}
)";
    auto sw = ParseObject<ScaleWidget>(
        Util::ReplaceString(input, "<UMM>",
                            use_modified_mode ? "True" : "False"));
    EXPECT_NOT_NULL(sw);
    return sw;
}

TEST_F(ScaleWidgetTest, Defaults) {
    auto sw = GetScaleWidget();

    EXPECT_EQ(ScaleWidget::Mode::kAsymmetric, sw->GetMode());
    EXPECT_FALSE(sw->IsUsingModifiedMode());
    EXPECT_EQ(Vector2f(TK::kScaleWidgetMinLimit,
                       TK::kScaleWidgetMaxLimit), sw->GetLimits());
    EXPECT_EQ(0, sw->GetMinValue());
    EXPECT_EQ(0, sw->GetMaxValue());
    EXPECT_EQ(0, sw->GetLength());

    EXPECT_NOT_NULL(sw->GetMinSlider());
    EXPECT_NOT_NULL(sw->GetMaxSlider());
    EXPECT_EQ(0, sw->GetMinSlider()->GetValue());
    EXPECT_EQ(0, sw->GetMaxSlider()->GetValue());
}

TEST_F(ScaleWidgetTest, Set) {
    auto sw = GetScaleWidget(true);

    // Set the ranges on the min/max Slider1DWidget instances so the values
    // below will work.
    sw->GetMinSlider()->SetRange(-4000, 4000);
    sw->GetMaxSlider()->SetRange(-4000, 4000);

    sw->SetMode(ScaleWidget::Mode::kSymmetric);
    EXPECT_EQ(ScaleWidget::Mode::kSymmetric, sw->GetMode());

    EXPECT_TRUE(sw->IsUsingModifiedMode());

    sw->SetLimits(Vector2f(.1f, 1000));
    EXPECT_EQ(Vector2f(.1f, 1000), sw->GetLimits());

    sw->SetMinValue(-10);
    sw->SetMaxValue(123);
    EXPECT_EQ(-10, sw->GetMinValue());
    EXPECT_EQ(123, sw->GetMaxValue());
    EXPECT_EQ(133, sw->GetLength());

    sw->SetMaxValue(10);
    sw->SetMinValue(9.99f);   // Too small a length.
    EXPECT_EQ(10 - .1f, sw->GetMinValue());

    sw->SetMinValue(-10);
    sw->SetMaxValue(2000);   // Too large a length.
    EXPECT_EQ(990,  sw->GetMaxValue());
    EXPECT_EQ(1000, sw->GetLength());
}

TEST_F(ScaleWidgetTest, DragAsymmetric) {
    auto sw = GetScaleWidget();
    sw->GetMinSlider()->SetRange(-100, 100);
    sw->GetMaxSlider()->SetRange(-100, 100);

    size_t min_count = 0;
    size_t max_count = 0;
    sw->GetScaleChanged().AddObserver("key", [&](Widget &, bool is_max){
        if (is_max)
            ++max_count;
        else
            ++min_count;
    });

    DragTester dtmin(sw, std::vector<std::string>{ "MinSlider" });
    DragTester dtmax(sw, std::vector<std::string>{ "MaxSlider" });

    // Scale by dragging the min slider.
    dtmin.ApplyMouseDrag(Point3f(4, 0, 0), Point3f(-2, 0, 0));
    EXPECT_EQ(-6, sw->GetMinValue());
    EXPECT_EQ(0,  sw->GetMaxValue());
    EXPECT_EQ(6,  sw->GetLength());
    EXPECT_EQ(1U, min_count);
    EXPECT_EQ(0U, max_count);

    // Scale by dragging the min slider.
    dtmax.ApplyMouseDrag(Point3f(1, 0, 0), Point3f(8, 0, 0));
    EXPECT_EQ(-6, sw->GetMinValue());
    EXPECT_EQ(7,  sw->GetMaxValue());
    EXPECT_EQ(13, sw->GetLength());
    EXPECT_EQ(1U, min_count);
    EXPECT_EQ(1U, max_count);
}

TEST_F(ScaleWidgetTest, DragSymmetric) {
    auto sw = GetScaleWidget(true);
    sw->GetMinSlider()->SetRange(-100, 100);
    sw->GetMaxSlider()->SetRange(-100, 100);

    size_t min_count = 0;
    size_t max_count = 0;
    sw->GetScaleChanged().AddObserver("key", [&](Widget &, bool is_max){
        if (is_max)
            ++max_count;
        else
            ++min_count;
    });

    DragTester dtmin(sw, std::vector<std::string>{ "MinSlider" });
    DragTester dtmax(sw, std::vector<std::string>{ "MaxSlider" });
    dtmin.SetIsModifiedMode(true);
    dtmax.SetIsModifiedMode(true);

    // Scale by dragging the min slider.
    dtmin.ApplyMouseDrag(Point3f(4, 0, 0), Point3f(-2, 0, 0));
    EXPECT_EQ(-6, sw->GetMinValue());
    EXPECT_EQ( 6, sw->GetMaxValue());
    EXPECT_EQ(12, sw->GetLength());
    EXPECT_EQ(1U, min_count);
    EXPECT_EQ(0U, max_count);

    // Scale by dragging the min slider.
    dtmax.ApplyMouseDrag(Point3f(1, 0, 0), Point3f(8, 0, 0));
    EXPECT_EQ(-13, sw->GetMinValue());
    EXPECT_EQ( 13, sw->GetMaxValue());
    EXPECT_EQ(26,  sw->GetLength());
    EXPECT_EQ(1U,  min_count);
    EXPECT_EQ(1U,  max_count);
}
