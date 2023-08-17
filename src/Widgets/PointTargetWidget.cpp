#include "Widgets/PointTargetWidget.h"

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>

#include "SG/CoordConv.h"
#include "SG/Search.h"
#include "SG/Tube.h"
#include "Util/Assert.h"
#include "Widgets/RadialLayoutWidget.h"

void PointTargetWidget::AddFields() {
    AddField(target_.Init("target"));

    TargetWidgetBase::AddFields();
}

bool PointTargetWidget::IsValid(Str &details) {
    if (! TargetWidgetBase::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
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
        feedback_line_  = SG::FindTypedShapeInNode<SG::Tube>(*feedback_,
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

void PointTargetWidget::StartTargetPlacement() {
    TargetWidgetBase::StartTargetPlacement();

    // Turn off the RadialLayoutWidget to avoid confusion.
    layout_widget_->SetEnabled(false);
}

void PointTargetWidget::EndTargetPlacement() {
    TargetWidgetBase::EndTargetPlacement();

    // Make sure the snap indicator is off.
    snap_indicator_->SetEnabled(false);

    // Turn the RadialLayoutWidget back on.
    layout_widget_->SetEnabled(true);
}

void PointTargetWidget::ShowExtraSnapFeedback(bool is_snapping) {
    TargetWidgetBase::ShowExtraSnapFeedback(is_snapping);

    if (is_snapping) {
        feedback_->SetBaseColor(GetActiveColor());
        // The end point is in stage coordinates, and so is the target. The
        // feedback needs to be converted to object coordinates of the target.
        const Matrix4f som = ion::math::Inverse(GetModelMatrix());
        feedback_line_->SetEndpoints(Point3f::Zero(), som * line_end_pt_);
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
    // Pass the activation along. This will update the object-to-world matrix
    // in this instance, which can then be passed to the RadialLayoutWidget.
    GetActivation().Notify(*this, is_activation);

    if (is_activation) {
        // Pass the RadialLayoutWidget a matrix to convert from stage to world
        // coordinates and its current rotation. These are used to orient text.
        layout_widget_->SetTextOrientation(GetStageToWorldMatrix(),
                                           GetRotation());
    }
}

void PointTargetWidget::LayoutWidgetChanged_() {
    // Update the values in the PointTarget.
    auto &target = *target_.GetValue();
    target.SetRadius(layout_widget_->GetRadius());
    target.SetArc(layout_widget_->GetArc());

    // Pass along the notification.
    GetChanged().Notify(*this);
}
