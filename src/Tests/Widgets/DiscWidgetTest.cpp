#include "Parser/Exception.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/WidgetTestBase.h"
#include "Widgets/DiscWidget.h"

class DiscWidgetTest : public WidgetTestBase {};

TEST_F(DiscWidgetTest, Defaults) {
    auto dw = CreateObject<DiscWidget>();

    // Modes.
    EXPECT_EQ(DiscWidget::Mode::kRotateAndScale, dw->GetMode());
    EXPECT_EQ(DiscWidget::AngleMode::kClamp,     dw->GetAngleMode());

    // Transforms.
    EXPECT_EQ(1,                 dw->GetScaleFactor());
    EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
    EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
}

TEST_F(DiscWidgetTest, RotationInField) {
    TEST_THROW(ReadTypedItem<DiscWidget>("DiscWidget { rotation: 0 1 0 10 }"),
               Parser::Exception, "Rotation will be ignored");
}

TEST_F(DiscWidgetTest, NoMotion) {
    auto dw = CreateObject<DiscWidget>();

    DragTester dt(dw);
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.ApplyMouseDrag(Point3f(0, 0, 2), Point3f(0.0001f, 0, 2));
    EXPECT_EQ(1,                 dw->GetScaleFactor());
    EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
    EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
}

TEST_F(DiscWidgetTest, DragScale) {
    auto dw = CreateObject<DiscWidget>();

    // Dragging from Z=2 to Z=4 should scale by 2.
    DragTester dt(dw);
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.ApplyMouseDrag(Point3f(0, 0, 2), Point3f(0, 0, 4));
    EXPECT_EQ(2,                 dw->GetScaleFactor());
    EXPECT_EQ(Vector3f(2, 2, 2), dw->GetScale());
    EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());

    // Dragging from X=3 to X=9 should scale by 3. Add some intermediate points
    // to make sure it works.
    dt.ApplyMouseDrag(Point3f(3, 0, 0), Point3f(9, 0, 0), 4);
    EXPECT_EQ(3,                 dw->GetScaleFactor());
    EXPECT_EQ(Vector3f(6, 6, 6), dw->GetScale());
    EXPECT_EQ(0,                 dw->GetRotationAngle().Degrees());
}

TEST_F(DiscWidgetTest, DragRotate) {
    auto dw = CreateObject<DiscWidget>();

    // Dragging along +X offset from center should rotate counterclockwise.
    DragTester dt(dw);
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.ApplyMouseDrag(Point3f(0, 0, 2), Point3f(2, 0, 2));
    EXPECT_EQ(1,                 dw->GetScaleFactor());
    EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
    EXPECT_EQ(45,                dw->GetRotationAngle().Degrees());
}

TEST_F(DiscWidgetTest, DragEdgeOnRotate) {
    auto dw = CreateObject<DiscWidget>();

    // Dragging along +X with the ray along -Z should do edge-on rotation.
    DragTester dt(dw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(-4, 0, 4), Point3f(4, 0, 4));
    EXPECT_EQ(1,                 dw->GetScaleFactor());
    EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
    EXPECT_EQ(90,                dw->GetRotationAngle().Degrees());

    // Same test, but drag off the cylinder.
    dt.ApplyMouseDrag(Point3f(-4, 0, 4), Point3f(10, 0, 4), 4);
    EXPECT_EQ(1,                 dw->GetScaleFactor());
    EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
    EXPECT_CLOSE(186.0612f,      dw->GetRotationAngle().Degrees());
}

TEST_F(DiscWidgetTest, DragScaleOnly) {
    auto dw = ReadTypedItem<DiscWidget>("DiscWidget { mode: \"kScaleOnly\" }");
    EXPECT_EQ(DiscWidget::Mode::kScaleOnly, dw->GetMode());

    // Dragging from Z=2 to Z=4 should scale by 2.
    DragTester dt(dw);
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.ApplyMouseDrag(Point3f(0, 0, 2), Point3f(0, 0, 4));
    EXPECT_EQ(2,                 dw->GetScaleFactor());
    EXPECT_EQ(Vector3f(2, 2, 2), dw->GetScale());
}

TEST_F(DiscWidgetTest, DragRotateOnly) {
    auto dw =
        ReadTypedItem<DiscWidget>("DiscWidget { mode: \"kRotationOnly\" }");

    // Dragging along +X offset from center should rotate counterclockwise.
    DragTester dt(dw);
    dt.SetRayDirection(-Vector3f::AxisY());
    dt.ApplyMouseDrag(Point3f(0, 0, 2), Point3f(2, 0, 2));
    EXPECT_EQ(1,                 dw->GetScaleFactor());
    EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
    EXPECT_EQ(45,                dw->GetRotationAngle().Degrees());
}

TEST_F(DiscWidgetTest, GripRotate) {
    auto dw = CreateObject<DiscWidget>();

    DragTester dt(dw);
    dt.ApplyGripRotationDrag(Vector3f::AxisX(),
                             BuildRotation(1, 0, 0, 20),
                             BuildRotation(1, 0, 0, 100));
    EXPECT_EQ(1,                 dw->GetScaleFactor());
    EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
    EXPECT_CLOSE(80,             dw->GetRotationAngle().Degrees());

    // Repeat with opposite guide direction.
    dw->SetRotationAngle(Anglef::FromDegrees(0));
    dt.ApplyGripRotationDrag(-Vector3f::AxisX(),
                             BuildRotation(1, 0, 0, 20),
                             BuildRotation(1, 0, 0, 100));
    EXPECT_EQ(1,                 dw->GetScaleFactor());
    EXPECT_EQ(Vector3f(1, 1, 1), dw->GetScale());
    EXPECT_CLOSE(-80,            dw->GetRotationAngle().Degrees());
}

TEST_F(DiscWidgetTest, ClampAngle) {
    auto dw = CreateObject<DiscWidget>();

    // This is easier to test using grip angles directly.

    DragTester dt(dw);

    auto test_rot = [&](float expected_deg, float rot_deg0, float rot_deg1){
        dw->SetRotationAngle(Anglef::FromDegrees(0));  // Start clean.
        dt.ApplyGripRotationDrag(Vector3f::AxisX(),
                                 BuildRotation(1, 0, 0, 0),
                                 BuildRotation(1, 0, 0, rot_deg0),
                                 BuildRotation(1, 0, 0, rot_deg1));
        EXPECT_CLOSE(expected_deg, dw->GetRotationAngle().Degrees());
    };

    // Start in the positive direction.
    test_rot(240, 120, -120);   // > 180 difference.

    // Start in the negative direction.
    test_rot(-220, -130, 140);  // < -180 difference.
}

TEST_F(DiscWidgetTest, AccumulateAngle) {
    auto dw =
        ReadTypedItem<DiscWidget>("DiscWidget { angle_mode: \"kAccumulate\" }");

    // Similar to the above test, but the angle should accumulate.
    // Positive direction.
    DragTester dt(dw);

    auto test_rot = [&](float expected_deg, float rot_deg0, float rot_deg1){
        dw->SetRotationAngle(Anglef::FromDegrees(0));  // Start clean.
        dt.ApplyGripRotationDrag(Vector3f::AxisX(),
                                 BuildRotation(1, 0, 0, 0),
                                 BuildRotation(1, 0, 0, rot_deg0),
                                 BuildRotation(1, 0, 0, rot_deg1));
        EXPECT_CLOSE(expected_deg, dw->GetRotationAngle().Degrees());
    };

    // Positive direction.
    test_rot(170,  100,  170);
    test_rot(200,  170, -160);  // Has to switch angle sign.

    // Negative direction.
    test_rot(-160,  -40, -160);
    test_rot(-210, -170,  150);  // Has to switch angle sign.
}
