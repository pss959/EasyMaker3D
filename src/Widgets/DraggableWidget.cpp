#include "Widgets/DraggableWidget.h"

#include <ion/math/vectorutils.h>

#include "Base/Tuning.h"
#include "Math/Linear.h"

DraggableWidget::DraggableWidget() :
    grip_drag_scale_(TK::kGripDragScale) {
}

void DraggableWidget::StartDrag(const DragInfo &info) {
    start_info_ = cur_info_ = info;
}

void DraggableWidget::ContinueDrag(const DragInfo &info) {
    cur_info_ = info;
}

SG::CoordConv DraggableWidget::GetCoordConv() const {
    return SG::CoordConv(start_info_.path_to_widget);
}

Point3f DraggableWidget::WidgetToWorld(const Point3f &p) const {
    return GetCoordConv().LocalToRoot(p);
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
