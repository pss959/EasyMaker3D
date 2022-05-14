#include "Widgets/RadialLayoutWidget.h"

#include "Math/Linear.h"
#include "SG/Search.h"
#include "SG/Torus.h"
#include "Widgets/DiscWidget.h"

#include <ion/math/angleutils.h>

// ----------------------------------------------------------------------------
// Constants for various parts of the widget.
// ----------------------------------------------------------------------------

namespace {

static const float kDefaultRadius_        =  1;
static const float kRingMinInnerRadius_   = .2f;
static const float kRingMaxInnerRadius_   = .3f;
static const int   kRingRingCount_        =  8;
static const int   kRingMinNumSectors_    = 36;
static const int   kRingMaxNumSectors_    = 72;
static const float kSpokeScale_           = 1.1f;  // Relative to radius.
static const float kMinRadiusForSpokes_   = 1.5f;
static const float kArcRadiusScale_       = .6f;   // Relative to radius.
static const float kArcDegreesPerSegment_ = 4;
static const float kArcLineWidth_         = .2f;
static const float kTextYOffset_          = 6;

static const std::string kDegreeSign_ = "\xb0";

}  // anonymous namespace

// ----------------------------------------------------------------------------
// RadialLayoutWidget functions.
// ----------------------------------------------------------------------------

void RadialLayoutWidget::CreationDone() {
    Widget::CreationDone();

    if (! IsTemplate()) {
        auto get_widget = [&](const std::string &name){
            return SG::FindTypedNodeUnderNode<DiscWidget>(*this, name);
        };

        // Main parts.
        ring_        = get_widget("Ring");
        start_spoke_ = get_widget("StartSpoke");
        end_spoke_   = get_widget("EndSpoke");
        arc_         = SG::FindNodeUnderNode(*this, "Arc");
#if XXXX

        // Feedback.

        // Text feedback.
        radius_text_ = SG::FindNodeUnderNode(*this, "Arc");



        feedback_       = SG::FindNodeUnderNode(*this, "Feedback");
        feedback_line_  = SG::FindTypedShapeInNode<SG::Line>(*feedback_,
                                                             "FeedbackLine");
        UpdateFromTarget_(GetPointTarget());
#endif
        // Set up callbacks.
        ring_->GetActivation().AddObserver(
            this, [&](Widget &, bool is_act){ start_radius_ = radius_; });
        ring_->GetScaleChanged().AddObserver(
            this, [&](Widget &, float change){ RadiusChanged_(change); });
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

void RadialLayoutWidget::RadiusChanged_(float change) {
    // XXXX _radius = Precision.ApplyPositive(_startRadius + change);
    radius_ = start_radius_ + .25f * change;
    UpdateRing_();
    UpdateSpokes_();
    // UpdateText_(_radiusText, 2f * _radius, _radiusWidget.transform.position);
    changed_.Notify();
}

void RadialLayoutWidget::UpdateRing_() {
    // Compute a reasonable inner radius and number of sectors based on the
    // current radius.
    const float inner_radius =
        Clamp(.1f * radius_, kRingMinInnerRadius_, kRingMaxInnerRadius_);
    const int sector_count =
        static_cast<int>(Clamp(30 * radius_,
                               kRingMinNumSectors_, kRingMaxNumSectors_));

    // Update the ring torus geometry.
    auto &torus = *SG::FindTypedShapeInNode<SG::Torus>(*ring_, "Torus");
    torus.SetGeometry(inner_radius, radius_, kRingRingCount_, sector_count);
}

void RadialLayoutWidget::UpdateSpokes_() {
    // XXXX
}
