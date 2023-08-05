#include "Tests/Widgets/TargetWidgetTestBase.h"

TargetWidgetTestBase::TargetWidgetTestBase() {
        Parser::Registry::AddType<TestTargetWidget>("TestTargetWidget");
}

void TargetWidgetTestBase::TestTargetWidget::PlacePointTarget(
    const DragInfo &info, Point3f &position, Vector3f &direction,
    Dimensionality &snapped_dims) {
    position     = Point3f(1, 2, 3);
    direction    = Vector3f(-1, 0, 0);
    snapped_dims = Dimensionality("XZ");
}

void TargetWidgetTestBase::TestTargetWidget::PlaceEdgeTarget(
    const DragInfo &info, float current_length,
    Point3f &position0, Point3f &position1) {
    position0 = Point3f(1, 2, 3);
    position1 = Point3f(4, 5, 6);
}
