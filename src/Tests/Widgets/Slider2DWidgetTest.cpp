//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Testing.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/Assert.h"
#include "Widgets/Slider2DWidget.h"

/// \ingroup Tests
class Slider2DWidgetTest : public SceneTestBase {};

TEST_F(Slider2DWidgetTest, Defaults) {
    auto sw = CreateObject<Slider2DWidget>();

    EXPECT_EQ(Slider2DWidget::PrincipalPlane::kXY, sw->GetPrincipalPlane());
    EXPECT_FALSE(sw->IsPrecisionBased());
    EXPECT_FALSE(sw->IsNormalized());
    EXPECT_EQ(Vector2f(0, 0), sw->GetMinValue());
    EXPECT_EQ(Vector2f(1, 1), sw->GetMaxValue());
    EXPECT_EQ(Vector2f(0, 0), sw->GetInitialValue());
    EXPECT_EQ(Vector2f(0, 0), sw->GetUnnormalizedValue());
}

TEST_F(Slider2DWidgetTest, Planes) {
    auto sw = ParseTypedObject<Slider2DWidget>(
        R"(Slider2DWidget { principal_plane: "kXZ" })");

    EXPECT_EQ(Slider2DWidget::PrincipalPlane::kXZ, sw->GetPrincipalPlane());

    sw = ParseTypedObject<Slider2DWidget>(
        R"(Slider2DWidget { principal_plane: "kYZ" })");
    EXPECT_EQ(Slider2DWidget::PrincipalPlane::kYZ, sw->GetPrincipalPlane());
}

TEST_F(Slider2DWidgetTest, Set) {
    auto sw = ParseTypedObject<Slider2DWidget>(
        "Slider2DWidget { is_normalized: True, initial_value: 1.3 4.2 }");

    EXPECT_TRUE(sw->IsNormalized());
    EXPECT_EQ(Vector2f(1.3f, 4.2f), sw->GetInitialValue());

    sw->SetIsPrecisionBased(true);
    EXPECT_TRUE(sw->IsPrecisionBased());

    sw->SetRange(Vector2f(-2, -3), Vector2f(-1.5f, 12.5f));
    EXPECT_EQ(Vector2f(-2,    -3),    sw->GetMinValue());
    EXPECT_EQ(Vector2f(-1.5f, 12.5f), sw->GetMaxValue());

    sw->SetInitialValue(Vector2f(2.5f, -1.4f));
    EXPECT_EQ(Vector2f(2.5f, -1.4f), sw->GetInitialValue());
}

TEST_F(Slider2DWidgetTest, Drag) {
    auto sw = CreateObject<Slider2DWidget>();
    sw->SetRange(Vector2f(-10, -10), Vector2f(10, 10));

    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6, 1.5f, 2));
    EXPECT_EQ(Vector2f(6, .5f), sw->GetValue());
}

TEST_F(Slider2DWidgetTest, DragNormalized) {
    auto sw = ParseTypedObject<Slider2DWidget>(
        "Slider2DWidget { is_normalized: True }");
    sw->SetRange(Vector2f(-10, -10), Vector2f(10, 10));

    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6, 1.5f, 2));
    EXPECT_EQ(Vector2f(6, .5f) / 20, sw->GetValue());
}

TEST_F(Slider2DWidgetTest, DragPrecisionBased) {
    auto sw = CreateObject<Slider2DWidget>();
    sw->SetIsPrecisionBased(true);
    sw->SetRange(Vector2f(-10, -10), Vector2f(10, 10));

    DragTester dt(sw);
    dt.SetLinearPrecision(.1f);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(6.5f, 1.5f, 2), 4);
    EXPECT_EQ(.1f * Vector2f(6.5f, .5f), sw->GetValue());
}

TEST_F(Slider2DWidgetTest, Parallel) {
    auto sw = CreateObject<Slider2DWidget>();
    sw->SetRange(Vector2f(-10, -10), Vector2f(10, 10));

    DragTester dt(sw);
    dt.SetRayDirection(Vector3f::AxisX());
    dt.ApplyMouseDrag(Point3f(2, 3, 2), Point3f(4, 5, 2));
    EXPECT_EQ(Vector2f(0, 0), sw->GetValue());
}

TEST_F(Slider2DWidgetTest, GripDrag) {
    auto sw = CreateObject<Slider2DWidget>();
    sw->SetRange(Vector2f(-10, -10), Vector2f(10, 10));

    // Use small motion in X to stay within range.
    DragTester dt(sw);
    dt.ApplyGripDrag(Point3f(0, 0, 0), Point3f(.1f, -.1f, .3f));
    EXPECT_EQ(Vector2f(.1f, -.1f) * sw->GetGripDragScale(), sw->GetValue());
}

TEST_F(Slider2DWidgetTest, TouchDrag) {
    auto sw = CreateObject<Slider2DWidget>();
    sw->SetRange(Vector2f(-10, -10), Vector2f(10, 10));

    DragTester dt(sw);
    dt.ApplyTouchDrag(Point3f(0, 0, 0), Point3f(2.5f, .2f, .3f));
    EXPECT_EQ(Vector2f(2.5f, .2f), sw->GetValue());
}

TEST_F(Slider2DWidgetTest, Notify) {
    auto sw = CreateObject<Slider2DWidget>();
    sw->SetRange(Vector2f(-10, -10), Vector2f(10, 10));

    std::vector<Vector2f> vals;
    sw->GetValueChanged().AddObserver("key", [&](Widget &, const Vector2f &val){
        vals.push_back(val);
    });

    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 1, 2), Point3f(4, 9, 2), 3);
    EXPECT_EQ(Vector2f(4, 8), sw->GetValue());

    // Should have notified 4 times. (4 value changes.)
    EXPECT_EQ(4U, vals.size());
    EXPECT_EQ(Vector2f(1, 2), vals[0]);
    EXPECT_EQ(Vector2f(2, 4), vals[1]);
    EXPECT_EQ(Vector2f(3, 6), vals[2]);
    EXPECT_EQ(Vector2f(4, 8), vals[3]);
}
