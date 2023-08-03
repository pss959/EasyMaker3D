#include "Tests/Widgets/WidgetTestBase.h"

#include "Math/Linear.h"
#include "Place/DragInfo.h"
#include "Widgets/DraggableWidget.h"

WidgetTestBase::DragTester::DragTester() {
    base_info_.ray.direction        = -Vector3f::AxisZ();
    base_info_.grip_guide_direction = -Vector3f::AxisX();
    base_info_.grip_position        =  Point3f::Zero();
}

void WidgetTestBase::DragTester::SetLinearPrecision(float p) {
    base_info_.linear_precision  = p;
}

void WidgetTestBase::DragTester::SetAngularPrecision(float p) {
    base_info_.angular_precision = p;
}

void WidgetTestBase::DragTester::SetPathToWidget(const SG::NodePath &path) {
    base_info_.path_to_widget = path;
    base_info_.hit.path       = path;
}

void WidgetTestBase::DragTester::SetRayDirection(const Vector3f &dir) {
    base_info_.ray.direction = dir;
}

void WidgetTestBase::DragTester::ApplyMouseDrag(DraggableWidget &dw,
                                                const Point3f &p0,
                                                const Point3f &p1,
                                                size_t count_between) {
    DragInfo info = base_info_;
    info.trigger  = Trigger::kPointer;

    std::vector<DragInfo> infos;

    const SG::CoordConv cc(info.path_to_widget);

    // Sets the given point to store in the SG::Hit and also computes the origin
    // point for the Ray based on the Ray direction. Adds the resulting
    // DragInfo to the infos vector.
    const auto add_pt = [&](const Point3f &p){
        info.hit.point = p;
        info.ray.origin = cc.LocalToRoot(p) - 10 * info.ray.direction;
        infos.push_back(info);
    };

    add_pt(p0);
    const float delta = 1.f / (count_between + 1);
    for (size_t i = 0; i < count_between; ++i)
        add_pt(Lerp((i + 1) * delta, p0, p1));
    add_pt(p1);

    ApplyDrag_(dw, infos);
}

void WidgetTestBase::DragTester::ApplyGripRotationDrag(
    DraggableWidget &dw, const Vector3f &guide_dir, const Rotationf &r0,
    const Rotationf &r1, size_t count_between) {
    ApplyGripRotationDrag(dw, guide_dir, r0, r1, r1, count_between);
}

void WidgetTestBase::DragTester::ApplyGripRotationDrag(
    DraggableWidget &dw, const Vector3f &guide_dir, const Rotationf &r0,
    const Rotationf &r1, const Rotationf &r2, size_t count_between) {
    DragInfo info = base_info_;
    info.trigger  = Trigger::kGrip;
    info.grip_guide_direction = guide_dir;

    std::vector<DragInfo> infos;

    // Sets the given point to store in the SG::Hit and also computes the
    // origin point for the Ray based on the Ray direction. Adds the resulting
    // DragInfo to the infos vector.
    const auto add_rot = [&](const Rotationf &r){
        info.grip_orientation = r;
        infos.push_back(info);
    };

    add_rot(r0);
    const float delta = 1.f / (count_between + 1);
    for (size_t i = 0; i < count_between; ++i)
        add_rot(Rotationf::Slerp(r0, r1, (i + 1) * delta));
    add_rot(r1);
    if (r2 != r1) {
        for (size_t i = 0; i < count_between; ++i)
            add_rot(Rotationf::Slerp(r1, r2, (i + 1) * delta));
        add_rot(r2);
    }

    ApplyDrag_(dw, infos);
}

void WidgetTestBase::DragTester::ApplyDrag_(DraggableWidget &dw,
                                            const std::vector<DragInfo> &infos) {
    ASSERT_LE(2U, infos.size());
    dw.StartDrag(infos[0]);
    for (size_t i = 1; i < infos.size(); ++i)
        dw.ContinueDrag(infos[i]);
    dw.EndDrag();
}
