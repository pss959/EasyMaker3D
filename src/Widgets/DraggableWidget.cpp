#include "Widgets/DraggableWidget.h"

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"

DraggableWidget::DraggableWidget() :
    grip_drag_scale_(TK::kGripDragScale) {
}

void DraggableWidget::StartDrag(const DragInfo &info) {
    KLOG('W', GetDesc() << " StartDrag");
    is_dragging_ = true;
    start_info_ = cur_info_ = info;
    dragged_.Notify(&info, true);
}

void DraggableWidget::ContinueDrag(const DragInfo &info) {
    cur_info_ = info;
    dragged_.Notify(&info, false);
}

void DraggableWidget::EndDrag() {
    KLOG('W', GetDesc() << " EndDrag");
    is_dragging_ = false;
    dragged_.Notify(nullptr, false);
}

void DraggableWidget::UpdateHoverPoint(const Point3f &point) {
    hovered_.Notify(point);
}

SG::CoordConv DraggableWidget::GetCoordConv() const {
    return SG::CoordConv(start_info_.path_to_widget);
}

Vector3f DraggableWidget::WidgetToWorld(const Vector3f &v,
                                        bool normalize) const {
    const Vector3f nv = GetCoordConv().LocalToRoot(v);
    return normalize ? ion::math::Normalized(nv) : nv;
}

Point3f DraggableWidget::WorldToWidget(const Point3f &p) const {
    return GetCoordConv().RootToLocal(p);
}

Vector3f DraggableWidget::WorldToWidget(const Vector3f &v,
                                        bool normalize) const {
    const Vector3f nv =  GetCoordConv().RootToLocal(v);
    return normalize ? ion::math::Normalized(nv) : nv;
}

Ray DraggableWidget::WorldToWidget(const Ray &ray) const {
    return TransformRay(ray, GetCoordConv().GetRootToLocalMatrix());
}
