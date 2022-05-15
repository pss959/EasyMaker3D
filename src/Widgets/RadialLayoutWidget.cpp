#include "Widgets/RadialLayoutWidget.h"

#include "Math/Linear.h"
#include "SG/PolyLine.h"
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
static const int   kRingMinSectorCount_   = 36;
static const int   kRingMaxSectorCount_   = 72;
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
        layout_      = SG::FindNodeUnderNode(*this, "Layout");
        spoke_       = SG::FindNodeUnderNode(*this, "Spoke");
        start_spoke_ = get_widget("StartSpoke");
        end_spoke_   = get_widget("EndSpoke");
        arc_         = SG::FindNodeUnderNode(*this, "Arc");

        // Set up callbacks.
        auto ring_act = [&](Widget &, bool is_act){ start_radius_ = radius_; };
        auto ring_change = [&](Widget &w, float change){
            auto &dw = static_cast<DiscWidget &>(w);
            RadiusChanged_(change, dw.GetCurrentDragInfo().linear_precision);
        };
        auto spoke_change = [&](Widget &w, const Anglef &angle){
            auto &dw = static_cast<DiscWidget &>(w);
            SpokeChanged_(angle, &dw == start_spoke_.get(),
                          dw.GetCurrentDragInfo().angular_precision);
        };

        ring_->GetActivation().AddObserver(this, ring_act);
        ring_->GetScaleChanged().AddObserver(this, ring_change);
        start_spoke_->GetRotationChanged().AddObserver(this, spoke_change);
        end_spoke_->GetRotationChanged().AddObserver(this, spoke_change);

        // Layout is off until radius is scaled up large enough.
        layout_->SetEnabled(false);
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

void RadialLayoutWidget::RadiusChanged_(float change, float precision) {
    radius_ = RoundToPrecision(start_radius_ * change, precision);
    UpdateRing_();
    UpdateSpokes_();
    // XXXX UpdateText_(_radiusText, 2f * _radius, _radiusWidget.transform.position);
    changed_.Notify();
}

void RadialLayoutWidget::SpokeChanged_(const Anglef &angle, bool is_start,
                                       float precision) {
    auto round_angle = [precision](const Anglef &a){
        return Anglef::FromDegrees(RoundToPrecision(a.Degrees(), precision));
    };
    if (is_start) {
        const Anglef new_angle =
            round_angle(NormalizedAngle(start_angle_ + angle));
        start_spoke_->SetRotationAngle(new_angle - start_angle_);
        start_angle_ = new_angle;
    }
    else {
        arc_angle_ = round_angle(NormalizedAngle(angle));
        if (arc_angle_.Radians() == 0)
            arc_angle_ = Anglef::FromDegrees(360);
        end_spoke_->SetRotationAngle(arc_angle_);
    }
    UpdateArc_();
}

void RadialLayoutWidget::UpdateRing_() {
    // Compute a reasonable inner radius and number of sectors based on the
    // current radius.
    const float inner_radius =
        Clamp(.1f * radius_, kRingMinInnerRadius_, kRingMaxInnerRadius_);
    const int sector_count =
        static_cast<int>(Clamp(32 * radius_,
                               kRingMinSectorCount_, kRingMaxSectorCount_));

    // Update the ring torus geometry.
    auto &torus = *SG::FindTypedShapeInNode<SG::Torus>(*ring_, "Torus");
    torus.SetGeometry(inner_radius, radius_, kRingRingCount_, sector_count);
}

void RadialLayoutWidget::UpdateSpokes_() {
    // If the radius is small, don't show the layout items (spokes and arc).
    const bool large_enough = radius_ >= kMinRadiusForSpokes_;
    layout_->SetEnabled(large_enough);

    if (large_enough) {
        const float spoke_size = kSpokeScale_ * radius_;
        Vector3f scale = spoke_->GetScale();
        Vector3f trans = spoke_->GetTranslation();
        scale[0] = spoke_size;
        trans[0] = .5f * spoke_size;
        spoke_->SetScale(scale);
        spoke_->SetTranslation(trans);
    }
    UpdateArc_();
    // XXXX UpdateAngleText_();
}

void RadialLayoutWidget::UpdateArc_() {
    auto &line = *SG::FindTypedShapeInNode<SG::PolyLine>(*arc_, "Line");
    line.SetArcPoints(start_angle_, arc_angle_, kArcRadiusScale_ * radius_,
                      kArcDegreesPerSegment_);
}
