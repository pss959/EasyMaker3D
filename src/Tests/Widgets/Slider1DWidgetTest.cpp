#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/Assert.h"
#include "Widgets/Slider1DWidget.h"

/// \ingroup Tests
class Slider1DWidgetTest : public SceneTestBase {};

TEST_F(Slider1DWidgetTest, Defaults) {
    auto sw = CreateObject<Slider1DWidget>();

    EXPECT_EQ(0, sw->GetDimension());
    EXPECT_FALSE(sw->IsPrecisionBased());
    EXPECT_FALSE(sw->IsNormalized());
    EXPECT_EQ(0, sw->GetMinValue());
    EXPECT_EQ(1, sw->GetMaxValue());
    EXPECT_EQ(0, sw->GetInitialValue());
    EXPECT_EQ(0, sw->GetUnnormalizedValue());
}

TEST_F(Slider1DWidgetTest, Set) {
    auto sw = ParseTypedObject<Slider1DWidget>(
        "Slider1DWidget { is_normalized: True, initial_value: 4.2 }");

    EXPECT_TRUE(sw->IsNormalized());
    EXPECT_EQ(4.2f, sw->GetInitialValue());

    sw->SetDimension(2);
    EXPECT_EQ(2, sw->GetDimension());

    sw->SetIsPrecisionBased(true);
    EXPECT_TRUE(sw->IsPrecisionBased());

    sw->SetRange(-3, 12.5f);
    EXPECT_EQ(-3,    sw->GetMinValue());
    EXPECT_EQ(12.5f, sw->GetMaxValue());

    TEST_THROW(sw->SetDimension(3), AssertException, "dim");
}

TEST_F(Slider1DWidgetTest, Drag) {
    auto sw = CreateObject<Slider1DWidget>();
    sw->SetRange(-10, 10);

    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6, 1.5f, 2));
    EXPECT_EQ(6, sw->GetValue());
}

TEST_F(Slider1DWidgetTest, DragNormalized) {
    auto sw = ParseTypedObject<Slider1DWidget>(
        "Slider1DWidget { is_normalized: True }");
    sw->SetRange(-10, 10);

    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6, 1.5f, 2));
    EXPECT_EQ(6.f / 20, sw->GetValue());
}

TEST_F(Slider1DWidgetTest, DragPrecisionBased) {
    auto sw = CreateObject<Slider1DWidget>();
    sw->SetIsPrecisionBased(true);
    sw->SetRange(-10, 10);

    DragTester dt(sw);
    dt.SetLinearPrecision(.1f);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6.5f, 1.5f, 2), 4);
    EXPECT_EQ(.1f * 6.5f, sw->GetValue());
}

TEST_F(Slider1DWidgetTest, DragVertical) {
    auto sw = CreateObject<Slider1DWidget>();
    sw->SetRange(-10, 10);
    sw->SetDimension(1);

    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, -2, 2), Point3f(1, 3.5f, 3));
    EXPECT_EQ(5.5f, sw->GetValue());
}

TEST_F(Slider1DWidgetTest, Parallel) {
    auto sw = CreateObject<Slider1DWidget>();
    sw->SetRange(-10, 10);
    sw->SetDimension(2);

    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 0, 2), Point3f(0, 0, 3));
    EXPECT_EQ(0, sw->GetValue());
}

TEST_F(Slider1DWidgetTest, GripDrag) {
    auto sw = CreateObject<Slider1DWidget>();
    sw->SetRange(-10, 10);

    // Use small motion in X to stay within range.
    DragTester dt(sw);
    dt.ApplyGripDrag(Point3f(0, 0, 0), Point3f(.1f, .2f, .3f));
    EXPECT_EQ(.1f * sw->GetGripDragScale(), sw->GetValue());

    // Try changing the scale.
    sw->SetGripDragScale(10);
    EXPECT_EQ(10, sw->GetGripDragScale());
    sw->SetValue(0);
    dt.ApplyGripDrag(Point3f(0, 0, 0), Point3f(.1f, .2f, .3f));
    EXPECT_EQ(.1f * sw->GetGripDragScale(), sw->GetValue());
}

TEST_F(Slider1DWidgetTest, TouchDrag) {
    auto sw = CreateObject<Slider1DWidget>();
    sw->SetRange(-10, 10);

    DragTester dt(sw);
    dt.ApplyTouchDrag(Point3f(0, 0, 0), Point3f(2.5f, .2f, .3f));
    EXPECT_EQ(2.5f, sw->GetValue());
}

TEST_F(Slider1DWidgetTest, Notify) {
    auto sw = CreateObject<Slider1DWidget>();
    sw->SetRange(-10, 10);

    std::vector<float> vals;
    sw->GetValueChanged().AddObserver("key", [&](Widget &, float val){
        vals.push_back(val);
    });

    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(4, 1.5f, 2), 3);
    EXPECT_EQ(4, sw->GetValue());

    // Should have notified 4 times. (4 value changes.)
    EXPECT_EQ(4U, vals.size());
    EXPECT_EQ(1, vals[0]);
    EXPECT_EQ(2, vals[1]);
    EXPECT_EQ(3, vals[2]);
    EXPECT_EQ(4, vals[3]);
}
