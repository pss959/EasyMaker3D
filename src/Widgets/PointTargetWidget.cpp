#include "Widgets/PointTargetWidget.h"

#include "App/CoordConv.h"
#include "SG/Line.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Widgets/RadialLayoutWidget.h"

void PointTargetWidget::AddFields() {
    AddField(target_.Init("target"));

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
        layout_widget_  = SG::FindTypedNodeUnderNode<RadialLayoutWidget>(
            *this, "RadialLayoutWidget");
        snap_indicator_ = SG::FindNodeUnderNode(*this, "SnapIndicator");
        feedback_       = SG::FindNodeUnderNode(*this, "Feedback");
        feedback_line_  = SG::FindTypedShapeInNode<SG::Line>(*feedback_,
                                                             "FeedbackLine");
        UpdateFromTarget_(GetPointTarget());

        layout_widget_->GetActivation().AddObserver(
            this,
            [&](Widget &, bool is_act){ LayoutWidgetActivated_(is_act); });
        layout_widget_->GetChanged().AddObserver(
            this, [&](){ LayoutWidgetChanged_(); });
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
    UpdateLayoutWidget_();
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

void PointTargetWidget::UpdateLayoutWidget_() {
    const auto &target = GetPointTarget();
    layout_widget_->SetRadius(target.GetRadius());
    layout_widget_->SetArc(target.GetArc());
}

void PointTargetWidget::LayoutWidgetActivated_(bool is_activation) {
    // Pass the activation along.
    GetActivation().Notify(*this, is_activation);
}

void PointTargetWidget::LayoutWidgetChanged_() {
    // Update the values in the PointTarget.
    auto &target = *target_.GetValue();
    target.SetRadius(layout_widget_->GetRadius());
    target.SetArc(layout_widget_->GetArc());

    // Pass along the notification.
    GetChanged().Notify(*this);
}
