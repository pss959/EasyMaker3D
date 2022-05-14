#include "Widgets/RadialLayoutWidget.h"

#include "SG/Search.h"

#include <ion/math/angleutils.h>

void RadialLayoutWidget::CreationDone() {
    Widget::CreationDone();

    if (! IsTemplate()) {
#if XXXX
        // Main parts.
        ring_        = SG::FindNodeUnderNode(*this, "Ring");
        start_spoke_ = SG::FindNodeUnderNode(*this, "StartSpoke");
        end_spoke_   = SG::FindNodeUnderNode(*this, "EndSpoke");
        arc_         = SG::FindNodeUnderNode(*this, "Arc");

        // Feedback.

        // Text feedback.
        radius_text_ = SG::FindNodeUnderNode(*this, "Arc");



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
