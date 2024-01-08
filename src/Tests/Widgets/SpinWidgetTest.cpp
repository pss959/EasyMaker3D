#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/SpinWidget.h"

/// \ingroup Tests
class SpinWidgetTest : public SceneTestBase {
  protected:
    SpinWidgetPtr GetSpinWidget() {
        return ReadRealNode<SpinWidget>(
            R"(children: [<"nodes/Widgets/SpinWidget.emd">])", "SpinWidget");
    }
};

TEST_F(SpinWidgetTest, Defaults) {
    auto sw = GetSpinWidget();

    Spin spin;
    EXPECT_EQ(spin, sw->GetSpin());
}

TEST_F(SpinWidgetTest, Set) {
    auto sw = GetSpinWidget();

    Spin spin;
    spin.center = Point3f(1, 2, 3);
    spin.axis   = Vector3f(1, 0, 0);
    spin.angle  = Anglef::FromDegrees(20);
    spin.offset = 13;

    sw->SetSpin(spin);
    EXPECT_EQ(spin, sw->GetSpin());
}

TEST_F(SpinWidgetTest, SetSize) {
    auto sw = GetSpinWidget();

    // The size (radius) is exact in the X and Z directions.
    sw->SetSize(10, 8);
    const auto size = sw->GetScaledBounds().GetSize();

    sw->SetSize(20, 8);
    EXPECT_EQ(2 * size[0], sw->GetScaledBounds().GetSize()[0]);
    EXPECT_EQ(2 * size[2], sw->GetScaledBounds().GetSize()[2]);
}

TEST_F(SpinWidgetTest, Notify) {
    auto sw = GetSpinWidget();

    DragTester dtar(sw, StrVec{ "Axis", "Rotator"    });
    DragTester dtat(sw, StrVec{ "Axis", "Translator" });
    DragTester dtrr(sw, StrVec{ "Ring" });
    DragTester dtof(sw, StrVec{ "Offset" });

    SpinWidget::ChangeType last_type;
    Spin                   spin;

    sw->GetSpinChanged().AddObserver("key", [&](SpinWidget::ChangeType type){
        last_type = type;
        spin      = sw->GetSpin();
    });

    // These changes are in an order that avoids affecting other values.

    // Change offset.
    dtof.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(0, 2, 0));
    EXPECT_EQ(SpinWidget::ChangeType::kOffset, last_type);
    EXPECT_EQ(Vector3f(0, 1, 0), spin.axis);
    EXPECT_EQ(Point3f(0, 0, 0),  spin.center);
    EXPECT_EQ(0,                 spin.angle.Degrees());
    EXPECT_EQ(2,                 spin.offset);

    // Translate center.
    dtat.SetRayDirection(-Vector3f::AxisY());
    dtat.ApplyMouseDrag(Point3f(1, 0, 0), Point3f(5, 0, 0));
    EXPECT_EQ(SpinWidget::ChangeType::kCenter, last_type);
    EXPECT_EQ(Vector3f(0, 1, 0), spin.axis);
    EXPECT_EQ(Point3f(4, 0, 0),  spin.center);
    EXPECT_EQ(0,                 spin.angle.Degrees());
    EXPECT_EQ(2,                 spin.offset);

    // Change spin angle.
    dtrr.ApplyGripRotationDrag(Vector3f::AxisX(),
                               BuildRotation(1, 0, 0, 0),
                               BuildRotation(1, 0, 0, 45));
    EXPECT_EQ(SpinWidget::ChangeType::kAngle, last_type);
    EXPECT_EQ(Vector3f(0, 1, 0), spin.axis);
    EXPECT_EQ(Point3f(4, 0, 0),  spin.center);
    EXPECT_CLOSE(45,             spin.angle.Degrees());
    EXPECT_EQ(2,                 spin.offset);

    // Rotate axis.
    dtar.ApplyGripRotationDrag(Vector3f::AxisX(),
                               BuildRotation(1, 0, 0, 20),
                               BuildRotation(1, 0, 0, 110));
    EXPECT_EQ(SpinWidget::ChangeType::kAxis, last_type);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), spin.axis);
    EXPECT_EQ(Point3f(4, 0, 0),          spin.center);
    EXPECT_CLOSE(45,                     spin.angle.Degrees());
    EXPECT_EQ(2,                         spin.offset);
}
