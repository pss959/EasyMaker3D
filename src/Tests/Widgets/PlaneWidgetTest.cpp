#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/AxisWidget.h"
#include "Widgets/PlaneWidget.h"

/// \ingroup Tests
class PlaneWidgetTest : public SceneTestBase {
  protected:
    PlaneWidgetPtr GetPlaneWidget() {
        return ParseObject<PlaneWidget>(
            ReadResourceFile("nodes/Widgets/PlaneWidget.emd"));
    }
};

TEST_F(PlaneWidgetTest, Defaults) {
    auto pw = GetPlaneWidget();

    EXPECT_EQ(Plane(0, Vector3f(0, 1, 0)), pw->GetPlane());
}

TEST_F(PlaneWidgetTest, SetPlane) {
    auto pw = GetPlaneWidget();

    pw->SetPlane(Plane(1, Vector3f(0, 1, 0)));
    EXPECT_EQ(Plane(1, Vector3f(0, 1, 0)), pw->GetPlane());
}

TEST_F(PlaneWidgetTest, RotateAxis) {
    auto pw = GetPlaneWidget();

    // Dive into the PlaneWidget to get the actual DraggableWidget.
    DragTester dt(pw, std::vector<std::string>{ "Axis", "Rotator" });

    // Rotating around Y should have no effect on the plane.
    dt.ApplyGripRotationDrag(Vector3f::AxisX(),
                             BuildRotation(0, 1, 0, 20),
                             BuildRotation(0, 1, 0, 100));
    EXPECT_EQ(Plane(0, Vector3f(0, 1, 0)), pw->GetPlane());

    // Rotating around X should change the plane.
    dt.ApplyGripRotationDrag(Vector3f::AxisX(),
                             BuildRotation(1, 0, 0, 20),
                             BuildRotation(1, 0, 0, 110));
    EXPECT_EQ(0,                         pw->GetPlane().distance);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), pw->GetPlane().normal);
}

TEST_F(PlaneWidgetTest, Translate) {
    auto pw = GetPlaneWidget();

    // Dive into the PlaneWidget to get the actual DraggableWidget.
    DragTester dt(pw, std::vector<std::string>{ "PlaneTranslator" });

    pw->SetSize(2);
    pw->SetTranslationRange(Range1f(-10, 10));

    // Dragging in X should have no effect on the translation (along +Y).
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(4, 0, 0));
    EXPECT_EQ(Plane(0, Vector3f(0, 1, 0)), pw->GetPlane());

    // Dragging in Y should translate.
    dt.ApplyMouseDrag(Point3f(0, -4, 0), Point3f(0, 4, 0));
    EXPECT_EQ(8,                 pw->GetPlane().distance);
    EXPECT_EQ(Vector3f(0, 1, 0), pw->GetPlane().normal);

    // Make sure range is taken into account.
    dt.ApplyMouseDrag(Point3f(0, -4, 0), Point3f(0, 14, 0));
    EXPECT_EQ(10,                pw->GetPlane().distance);
    EXPECT_EQ(Vector3f(0, 1, 0), pw->GetPlane().normal);
}

TEST_F(PlaneWidgetTest, Notify) {
    auto pw = GetPlaneWidget();
    pw->SetTranslationRange(Range1f(-10, 10));

    size_t rot_count   = 0;
    size_t trans_count = 0;
    pw->GetPlaneChanged().AddObserver("key", [&](bool is_rot){
        if (is_rot)
            ++rot_count;
        else
            ++trans_count;
    });

    DragTester dtr(pw, std::vector<std::string>{ "Axis", "Rotator" });
    DragTester dtt(pw, std::vector<std::string>{ "PlaneTranslator" });

    // Rotate twice.
    dtr.ApplyGripRotationDrag(Vector3f::AxisX(),
                              BuildRotation(1, 0, 0, 20),
                              BuildRotation(1, 0, 0, 110));
    EXPECT_EQ(0,                         pw->GetPlane().distance);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), pw->GetPlane().normal);
    EXPECT_EQ(1U, rot_count);
    EXPECT_EQ(0U, trans_count);
    dtr.ApplyGripRotationDrag(Vector3f::AxisX(),
                              BuildRotation(0, 1, 0, 0),
                              BuildRotation(0, 1, 0, 90));
    EXPECT_EQ(0,                         pw->GetPlane().distance);
    EXPECT_VECS_CLOSE(Vector3f(1, 0, 0), pw->GetPlane().normal);
    EXPECT_EQ(2U, rot_count);
    EXPECT_EQ(0U, trans_count);

    // Translate twice. Note that the translation is in unrotated coordinates.
    dtt.ApplyMouseDrag(Point3f(0, -2, 0), Point3f(0, 4, 0));
    EXPECT_EQ(6,                         pw->GetPlane().distance);
    EXPECT_VECS_CLOSE(Vector3f(1, 0, 0), pw->GetPlane().normal);
    EXPECT_EQ(2U, rot_count);
    EXPECT_EQ(1U, trans_count);
    dtt.ApplyMouseDrag(Point3f(0, 1, 0), Point3f(0, -3, 0));
    EXPECT_EQ(2,                         pw->GetPlane().distance);
    EXPECT_VECS_CLOSE(Vector3f(1, 0, 0), pw->GetPlane().normal);
    EXPECT_EQ(2U, rot_count);
    EXPECT_EQ(2U, trans_count);
}
