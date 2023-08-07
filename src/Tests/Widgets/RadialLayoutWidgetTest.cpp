#include "Tests/Testing.h"
#include "Tests/Widgets/WidgetTestBase.h"
#include "Widgets/RadialLayoutWidget.h"
#include "Widgets/Slider2DWidget.h"
#include "Widgets/SphereWidget.h"

class RadialLayoutWidgetTest : public WidgetTestBase {
  protected:
    RadialLayoutWidgetPtr GetRadialLayoutWidget() {
        return GetWidgetOfType<RadialLayoutWidget>(
            "nodes/Widgets/RadialLayoutWidget.emd", "RadialLayoutWidget");
    }
};

TEST_F(RadialLayoutWidgetTest, Defaults) {
    auto rlw = GetRadialLayoutWidget();

    EXPECT_EQ(1,    rlw->GetRadius());
    EXPECT_EQ(0,    rlw->GetArc().start_angle.Degrees());
    EXPECT_EQ(-360, rlw->GetArc().arc_angle.Degrees());
}

TEST_F(RadialLayoutWidgetTest, Set) {
    auto rlw = GetRadialLayoutWidget();

    rlw->SetRadius(12);
    EXPECT_EQ(12, rlw->GetRadius());

    rlw->SetArc(CircleArc(Anglef::FromDegrees(20), Anglef::FromDegrees(-240)));
    EXPECT_CLOSE(20,   rlw->GetArc().start_angle.Degrees());
    EXPECT_CLOSE(-240, rlw->GetArc().arc_angle.Degrees());

    // No testable effects.
    rlw->SetTextOrientation(Matrix4f::Identity(), BuildRotation(0, 1, 0, 30));
}

TEST_F(RadialLayoutWidgetTest, Drag) {
    auto rlw = GetRadialLayoutWidget();

    size_t count = 0;
    rlw->GetChanged().AddObserver("key", [&](){ ++count; });

    DragTester dtr(rlw, std::vector<std::string>{ "Ring"       });
    DragTester dts(rlw, std::vector<std::string>{ "StartSpoke" });
    DragTester dte(rlw, std::vector<std::string>{ "EndSpoke"   });

    // Drag to change the radius.
    dtr.SetRayDirection(-Vector3f::AxisY());
    dtr.ApplyMouseDrag(Point3f(0, 0, 2), Point3f(0, 0, 4));
    EXPECT_EQ(1U, count);
    EXPECT_EQ(2, rlw->GetRadius());

    // Rotate the spokes.
    dts.ApplyGripRotationDrag(Vector3f::AxisX(),
                              BuildRotation(1, 0, 0, 20),
                              BuildRotation(1, 0, 0, 100));
    EXPECT_EQ(2U, count);
    EXPECT_CLOSE(80, rlw->GetArc().start_angle.Degrees());

    dte.ApplyGripRotationDrag(Vector3f::AxisX(),
                              BuildRotation(1, 0, 0, 20),
                              BuildRotation(1, 0, 0, 290));
    EXPECT_EQ(3U, count);
    EXPECT_CLOSE(-90, rlw->GetArc().arc_angle.Degrees());

    // Use precision to get exactly 360 and 0.
    dts.SetAngularPrecision(1);
    dts.ApplyGripRotationDrag(Vector3f::AxisX(),
                              BuildRotation(1, 0, 0, 20),
                              BuildRotation(1, 0, 0, 300));
    EXPECT_EQ(4U, count);
    EXPECT_CLOSE(0, rlw->GetArc().start_angle.Degrees());

    dte.SetAngularPrecision(1);
    dte.ApplyGripRotationDrag(Vector3f::AxisX(),
                              BuildRotation(1, 0, 0,  20),
                              BuildRotation(1, 0, 0, 110));
    EXPECT_EQ(5U, count);
    EXPECT_CLOSE(-360, rlw->GetArc().arc_angle.Degrees());

    // Cross over.
    dte.ApplyGripRotationDrag(Vector3f::AxisX(),
                              BuildRotation(1, 0, 0, 20),
                              BuildRotation(1, 0, 0, 40));
    EXPECT_EQ(6U, count);
    EXPECT_CLOSE(20, rlw->GetArc().arc_angle.Degrees());

    // Cross over again.
    dte.ApplyGripRotationDrag(Vector3f::AxisX(),
                              BuildRotation(1, 0, 0, 0),
                              BuildRotation(1, 0, 0, 190));
    dte.ApplyGripRotationDrag(Vector3f::AxisX(),
                              BuildRotation(1, 0, 0, 0),
                              BuildRotation(1, 0, 0, 190));
    EXPECT_CLOSE(40, rlw->GetArc().arc_angle.Degrees());

    // Cross over the other way.
    dte.ApplyGripRotationDrag(Vector3f::AxisX(),
                              BuildRotation(1, 0, 0, 20),
                              BuildRotation(1, 0, 0, -20));
    EXPECT_CLOSE(360, rlw->GetArc().arc_angle.Degrees());
}
