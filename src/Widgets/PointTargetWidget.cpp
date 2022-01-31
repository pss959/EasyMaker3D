#include "Widgets/PointTargetWidget.h"

#include "CoordConv.h"
#include "SG/Line.h"
#include "SG/Search.h"
#include "Util/Assert.h"

void PointTargetWidget::AddFields() {
    AddField(target_);
    TargetWidgetBase::AddFields();
}

bool PointTargetWidget::IsValid(std::string &details) {
    if (! TargetWidgetBase::IsValid(details))
        return false;
    if (! target_.GetValue()) {
        details = "Missing target";
        return false;
    }
    return true;
}

void PointTargetWidget::CreationDone() {
    TargetWidgetBase::CreationDone();

    if (! IsTemplate()) {
        // Find parts.
        snap_indicator_ = SG::FindNodeUnderNode(*this, "SnapIndicator");
        feedback_       = SG::FindNodeUnderNode(*this, "Feedback");
        feedback_line_  = SG::FindTypedShapeInNode<SG::Line>(*feedback_,
                                                             "FeedbackLine");
    }
}

void PointTargetWidget::SetPointTarget(const PointTarget &target) {
    target_.GetValue()->CopyFrom(target);
    UpdateFromTarget_(target);
}

void PointTargetWidget::StartDrag(const DragInfo &info) {
    // XXXX
    DraggableWidget::StartDrag(info);
    ASSERTM(! info.is_grip, "PointTargetWidget does not do grip drags");

    // Turn off intersections during the drag.
    SetEnabled(Flag::kIntersectAll, false);

    SetActive(true);
}

void PointTargetWidget::ContinueDrag(const DragInfo &info) {
    // If there is a Widget on the path that can receive a target, ask it where
    // to place the target.
    if (auto widget = GetReceiver(info)) {
        Point3f        position;
        Vector3f       direction;
        Dimensionality snapped_dims;
        widget->PlacePointTarget(info, position, direction, snapped_dims);

        // Update the PointTarget.
        auto &target = *target_.GetValue();
        target.SetPosition(position);
        target.SetDirection(direction);

        // Update the widget to match the target.
        UpdateFromTarget_(target);

        // Indicate snapping.
        SetSnapIndicator_(snapped_dims);

        NotifyChanged();
    }
}

void PointTargetWidget::EndDrag() {
    snap_indicator_->SetEnabled(SG::Node::Flag::kTraversal, false);
    SetActive(false);
    SetEnabled(Flag::kIntersectAll, true);
}

void PointTargetWidget::ShowExtraSnapFeedback(const CoordConv &cc,
                                              bool is_snapping) {
    if (is_snapping) {
        feedback_->SetBaseColor(GetActiveColor());
        // Convert the end point from stage coordinates to object coordinates.
        PointTargetWidgetPtr ptw = Util::CreateTemporarySharedPtr(this);
        const Point3f p = cc.StageToObject(SG::NodePath(ptw), line_end_pt_);
        feedback_line_->SetEndpoints(Point3f::Zero(), p);
    }
    feedback_->SetEnabled(SG::Node::Flag::kTraversal, is_snapping);
}

void PointTargetWidget::UpdateFromTarget_(const PointTarget &target) {
    SetTranslation(Vector3f(target.GetPosition()));
    SetRotation(Rotationf::RotateInto(Vector3f::AxisY(),
                                      target.GetDirection()));
}

void PointTargetWidget::SetSnapIndicator_(const Dimensionality &snapped_dims) {
    // Turn the indicator on or off.
    const bool is_on = snapped_dims.GetCount() > 0;
    snap_indicator_->SetEnabled(SG::Node::Flag::kTraversal, is_on);

    // Set the color if on.
    if (is_on) {
        Color color(0, 0, 0);
        for (int dim = 0; dim < 3; ++dim)
            if (snapped_dims.HasDimension(dim))
                color[dim] = 1;
        snap_indicator_->SetBaseColor(color);
    }
}
