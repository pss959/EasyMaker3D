//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Widgets/TargetWidgetBase.h"

#include "SG/ColorMap.h"
#include "SG/Intersector.h"

void TargetWidgetBase::CreationDone() {
    DraggableWidget::CreationDone();
    SetColorNamePrefix("Target");
}

void TargetWidgetBase::StartDrag(const DragInfo &info) {
    DraggableWidget::StartDrag(info);
    ASSERTM(info.trigger == Trigger::kPointer,
            GetTypeName() + " handles only pointer drags");

    // Turn off intersections during the drag.
    SetFlagEnabled(Flag::kIntersectAll, false);

    SetActive(true);

    // Let the derived class update anything it needs to.
    StartTargetPlacement();
}

void TargetWidgetBase::ContinueDrag(const DragInfo &info) {
    DraggableWidget::ContinueDrag(info);

    // If there is an ITargetable on the intersection path, this calls
    // PlaceTarget() to let the derived class place the target.

    // If modified-dragging, redo the intersection test with the bounds_only
    // flag set to true. This allows Models in the scene to have their bounds
    // intersected instead of their meshes.
    bool target_placed = false;
    if (info.is_modified_mode && info.trigger == Trigger::kPointer) {
        auto hit =
            SG::Intersector::IntersectGraph(info.hit.path[0], info.ray, true);
        if (auto targetable = hit.path.FindNodeUpwards<ITargetable>()) {
            if (targetable->CanTargetBounds()) {
                DragInfo bounds_info = info;
                bounds_info.hit = hit;
                PlaceTarget(*targetable, bounds_info);
                target_placed = true;
            }
        }
    }

    // If there was no bounds placement, try with the regular Hit info.
    if (! target_placed) {
        if (auto targetable = info.hit.path.FindNodeUpwards<ITargetable>()) {
            PlaceTarget(*targetable, info);
            target_placed = true;
        }
    }
    if (target_placed)
        NotifyChanged();
}

void TargetWidgetBase::EndDrag() {
    DraggableWidget::EndDrag();

    // Let the derived class update anything it needs to.
    EndTargetPlacement();

    SetActive(false);
    SetFlagEnabled(Flag::kIntersectAll, true);
}

void TargetWidgetBase::ShowSnapFeedback(bool is_snapping) {
    if (is_snapping == snap_feedback_active_)
        return;

    // Change the state of the Widget, but do not invoke the callbacks as it is
    // not being dragged.
    SetActive(is_snapping, false);

    // Let the derived class do anything extra it needs to do.
    ShowExtraSnapFeedback(is_snapping);

    snap_feedback_active_ = is_snapping;
}

Color TargetWidgetBase::GetActiveColor() {
    return SG::ColorMap::SGetColor("TargetActiveColor");
}
