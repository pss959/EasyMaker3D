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
        UpdateFromTarget_(GetPointTarget());
    }
}

void PointTargetWidget::SetPointTarget(const PointTarget &target) {
    target_.GetValue()->CopyFrom(target);
    UpdateFromTarget_(target);
}

void PointTargetWidget::PlaceTarget(Widget &widget, const DragInfo &info) {
    Point3f        position;
    Vector3f       direction;
    Dimensionality snapped_dims;
    widget.PlacePointTarget(info, position, direction, snapped_dims);

    // Update the PointTarget.
    auto &target = *target_.GetValue();
    target.SetPosition(position);
    target.SetDirection(direction);

    // Update the widget to match the target.
    UpdateFromTarget_(target);

    // Indicate snapping.
    SetSnapIndicator_(snapped_dims);
}

void PointTargetWidget::EndTargetPlacement() {
    snap_indicator_->SetEnabled(false);
}

void PointTargetWidget::ShowExtraSnapFeedback(const CoordConv &stage_cc,
                                              bool is_snapping) {
    if (is_snapping) {
        feedback_->SetBaseColor(GetActiveColor());
        // Convert the end point from stage coordinates to object coordinates
        // of this Widget.
        PointTargetWidgetPtr ptw = Util::CreateTemporarySharedPtr(this);
        const Point3f p = CoordConv(SG::NodePath(ptw)).RootToObject(
            stage_cc.ObjectToRoot(line_end_pt_));
        feedback_line_->SetEndpoints(Point3f::Zero(), p);
    }
    feedback_->SetEnabled(is_snapping);
}

void PointTargetWidget::UpdateFromTarget_(const PointTarget &target) {
    SetTranslation(target.GetPosition());
    SetRotation(Rotationf::RotateInto(Vector3f::AxisY(),
                                      target.GetDirection()));
}

void PointTargetWidget::SetSnapIndicator_(const Dimensionality &snapped_dims) {
    // Turn the indicator on or off.
    const bool is_on = snapped_dims.GetCount() > 0;
    snap_indicator_->SetEnabled(is_on);

    // Set the color if on.
    if (is_on) {
        Color color(0, 0, 0);
        for (int dim = 0; dim < 3; ++dim)
            if (snapped_dims.HasDimension(dim))
                color[dim] = 1;
        snap_indicator_->SetBaseColor(color);
    }
}
