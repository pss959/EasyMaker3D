#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/AxisWidget.h"
#include "Widgets/Slider2DWidget.h"
#include "Widgets/SphereWidget.h"

class AxisWidgetTest : public SceneTestBase {
  protected:
    AxisWidgetPtr GetAxisWidget() {
        return ParseObject<AxisWidget>(
            ReadResourceFile("nodes/Widgets/AxisWidget.emd"));
    }
};

TEST_F(AxisWidgetTest, Defaults) {
    auto aw = GetAxisWidget();

    EXPECT_EQ(Vector3f(0, 1, 0), aw->GetDirection());
    EXPECT_EQ(Point3f(0, 0, 0),  aw->GetPosition());
}

TEST_F(AxisWidgetTest, Set) {
    auto aw = GetAxisWidget();

    aw->SetPosition(Point3f(1, 2, 3));
    aw->SetDirection(Vector3f(1, 0, 0));

    EXPECT_VECS_CLOSE(Vector3f(1, 0, 0), aw->GetDirection());
    EXPECT_PTS_CLOSE(Point3f(1, 2, 3),   aw->GetPosition());
}

TEST_F(AxisWidgetTest, Rotate) {
    auto aw = GetAxisWidget();

    DragTester dt(aw, std::vector<std::string>{ "Rotator" });

    // Rotating around Y should have no effect on the direction.
    dt.ApplyGripRotationDrag(Vector3f::AxisX(),
                             BuildRotation(0, 1, 0, 20),
                             BuildRotation(0, 1, 0, 100));
    EXPECT_EQ(Vector3f(0, 1, 0), aw->GetDirection());
    EXPECT_EQ(Point3f(0, 0, 0),  aw->GetPosition());

    // Rotating around X should change the direction.
    dt.ApplyGripRotationDrag(Vector3f::AxisX(),
                             BuildRotation(1, 0, 0, 20),
                             BuildRotation(1, 0, 0, 110));
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), aw->GetDirection());
    EXPECT_EQ(Point3f(0, 0, 0),         aw->GetPosition());
}

TEST_F(AxisWidgetTest, Translate) {
    auto aw = GetAxisWidget();
    aw->SetTranslationRange(Range2f(Point2f(-100, -100), Point2f(100, 100)));

    DragTester dt(aw, std::vector<std::string>{ "Translator" });
    dt.SetRayDirection(-Vector3f::AxisY());
    EXPECT_EQ(Vector3f(0, 1, 0), aw->GetDirection());
    EXPECT_EQ(Point3f(0, 0, 0),  aw->GetPosition());

    // Dragging has no effect until translation is enabled.
    aw->SetTranslationEnabled(false);
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(10, 0, 0));
    EXPECT_EQ(Vector3f(0, 1, 0), aw->GetDirection());
    EXPECT_EQ(Point3f(0, 0, 0),  aw->GetPosition());
    aw->SetTranslationEnabled(true);

    // Dragging in Y should have no effect on the translation.
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(0, 4, 0));
    EXPECT_EQ(Vector3f(0, 1, 0), aw->GetDirection());
    EXPECT_EQ(Point3f(0, 0, 0),  aw->GetPosition());

    // Dragging in X or Z should translate.
    dt.ApplyMouseDrag(Point3f(-4, 0, 0), Point3f(3, 0, 0));
    EXPECT_EQ(Vector3f(0, 1, 0), aw->GetDirection());
    EXPECT_EQ(Point3f(7, 0, 0),  aw->GetPosition());
    dt.ApplyMouseDrag(Point3f(-1, 0, 1), Point3f(2, 0, 2));
    EXPECT_EQ(Vector3f(0, 1, 0), aw->GetDirection());
    EXPECT_EQ(Point3f(10, 0, 1), aw->GetPosition());
}
