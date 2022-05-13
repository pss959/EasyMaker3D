#include "Widgets/RadialLayoutWidget.h"

#include <ion/math/angleutils.h>

void RadialLayoutWidget::CreationDone() {
    Widget::CreationDone();

    if (! IsTemplate()) {
#if XXXX
        // Find parts.
        snap_indicator_ = SG::FindNodeUnderNode(*this, "SnapIndicator");
        feedback_       = SG::FindNodeUnderNode(*this, "Feedback");
        feedback_line_  = SG::FindTypedShapeInNode<SG::Line>(*feedback_,
                                                             "FeedbackLine");
        UpdateFromTarget_(GetPointTarget());
#endif
    }
}

void RadialLayoutWidget::SetRadius(float radius) {
    radius_ = radius;
    UpdateRing_();
    UpdateSpokes_();
}

void RadialLayoutWidget::SetAngles(const Anglef &start_angle,
                                   const Anglef &arc_angle) {
    start_angle_ = start_angle;
    arc_angle_   = arc_angle;
    UpdateSpokes_();
}

void RadialLayoutWidget::Reset() {
    radius_      = 1;
    start_angle_ = Anglef::FromDegrees(0);
    arc_angle_   = Anglef::FromDegrees(-360);

    UpdateRing_();
    UpdateSpokes_();
}

void RadialLayoutWidget::UpdateRing_() {
    // XXXX
}

void RadialLayoutWidget::UpdateSpokes_() {
    // XXXX
}
