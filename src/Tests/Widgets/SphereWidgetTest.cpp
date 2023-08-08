#include "Parser/Exception.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/SphereWidget.h"

class SphereWidgetTest : public SceneTestBase {};

TEST_F(SphereWidgetTest, Defaults) {
    auto sw = CreateObject<SphereWidget>();

    EXPECT_EQ(Rotationf::Identity(), sw->GetCurrentRotation());
}

TEST_F(SphereWidgetTest, NoMotion) {
    auto sw = CreateObject<SphereWidget>();

    // The intersection point is used for the sphere radius.
    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.00001f, 0, 0));

    EXPECT_EQ(Rotationf::Identity(), sw->GetCurrentRotation());
}

TEST_F(SphereWidgetTest, Drag) {
    auto sw = CreateObject<SphereWidget>();

    // The intersection point is used for the sphere radius.
    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(4, 0, 0));

    EXPECT_ROTS_CLOSE(BuildRotation(0, 1, 0, 207.846f),
                      sw->GetCurrentRotation());
}

TEST_F(SphereWidgetTest, GripDrag) {
    auto sw = CreateObject<SphereWidget>();

    DragTester dt(sw);
    dt.ApplyGripRotationDrag(Vector3f::AxisX(),
                             BuildRotation(0, 1, 0, 20),
                             BuildRotation(0, 1, 0, 100));
    EXPECT_ROTS_CLOSE(BuildRotation(0, 1, 0, 80), sw->GetCurrentRotation());

    // Accumulate.
    dt.ApplyGripRotationDrag(Vector3f::AxisX(),
                             BuildRotation(1, 0, 0, -10),
                             BuildRotation(1, 0, 0, -100));
    EXPECT_ROTS_CLOSE(BuildRotation(-1, 0, 0, 90), sw->GetCurrentRotation());
}

TEST_F(SphereWidgetTest, Notify) {
    auto sw = CreateObject<SphereWidget>();
    std::vector<Rotationf> rots;
    sw->GetRotationChanged().AddObserver(
        "key", [&](Widget &, const Rotationf &rot){ rots.push_back(rot); });

    DragTester dt(sw);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(-2, 0, 0), Point3f(2, 0, 0), 3);

    // Should have notified 4 times. (4 value changes.)
    const float deg = 207.846f;
    EXPECT_ROTS_CLOSE(BuildRotation(0, 1, 0, deg), sw->GetCurrentRotation());
    EXPECT_EQ(4U, rots.size());
    EXPECT_ROTS_CLOSE(BuildRotation(0, 1, 0, .25f * deg), rots[0]);
    EXPECT_ROTS_CLOSE(BuildRotation(0, 1, 0, .5f  * deg), rots[1]);
    EXPECT_ROTS_CLOSE(BuildRotation(0, 1, 0, .75f * deg), rots[2]);
    EXPECT_ROTS_CLOSE(BuildRotation(0, 1, 0,   1  * deg), rots[3]);
}
