#include "Widgets/RadialLayoutWidget.h"

#include "Math/Linear.h"
#include "SG/PolyLine.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "SG/Torus.h"
#include "Widgets/DiscWidget.h"

#include <ion/math/angleutils.h>

// ----------------------------------------------------------------------------
// Constants for various parts of the widget.
// ----------------------------------------------------------------------------

namespace {

static const float kDefaultRadius_         =  1;
static const float kRingMinInnerRadius_    = .2f;
static const float kRingMaxInnerRadius_    = .3f;
static const int   kRingRingCount_         =  8;
static const int   kRingMinSectorCount_    = 36;
static const int   kRingMaxSectorCount_    = 72;
static const float kSpokeScale_            = 1.1f;  // Relative to radius.
static const float kMinRadiusForSpokes_    = 1.5f;
static const float kArcRadiusScale_        = .6f;   // Relative to radius.
static const float kArcDegreesPerSegment_  = 4;
static const float kArcLineWidth_          = .2f;
static const float kRadiusTextYOffset_     = 1;
static const float kStartAngleTextYOffset_ = 1;
static const float kArcAngleTextYOffset_   = 1.6f;

static const std::string kDegreeSign_ = "°";

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
        auto get_text = [&](const std::string &name){
            return SG::FindTypedNodeUnderNode<SG::TextNode>(*this, name);
        };

        // Main parts.
        ring_             = get_widget("Ring");
        layout_           = SG::FindNodeUnderNode(*this, "Layout");
        spoke_geom_       = SG::FindNodeUnderNode(*this, "SpokeGeom");
        start_spoke_      = get_widget("StartSpoke");
        end_spoke_        = get_widget("EndSpoke");
        arc_              = SG::FindNodeUnderNode(*this, "Arc");
        radius_text_      = get_text("RadiusText");
        start_angle_text_ = get_text("StartAngleText");
        arc_angle_text_   = get_text("ArcAngleText");

        // Set up callbacks.
        auto ring_act = [&](Widget &, bool is_act){
            RadiusActivated_(is_act);
        };
        auto ring_change = [&](Widget &w, float change){
            auto &dw = static_cast<DiscWidget &>(w);
            RadiusChanged_(change, dw.GetCurrentDragInfo().linear_precision);
        };
        auto spoke_act = [&](Widget &w, bool is_act){
            SpokeActivated_(is_act, &w == start_spoke_.get());
        };
        auto spoke_change = [&](Widget &w, const Anglef &angle){
            auto &dw = static_cast<DiscWidget &>(w);
            SpokeChanged_(angle, &dw == start_spoke_.get(),
                          dw.GetCurrentDragInfo().angular_precision);
        };

        ring_->GetActivation().AddObserver(this, ring_act);
        ring_->GetScaleChanged().AddObserver(this, ring_change);
        start_spoke_->GetActivation().AddObserver(this, spoke_act);
        end_spoke_->GetActivation().AddObserver(this, spoke_act);
        start_spoke_->GetRotationChanged().AddObserver(this, spoke_change);
        end_spoke_->GetRotationChanged().AddObserver(this, spoke_change);

        // Layout is off until radius is scaled up large enough.
        layout_->SetEnabled(false);

        // Feedback is off until Widgets are activated.
        radius_text_->SetEnabled(false);
        start_angle_text_->SetEnabled(false);
        arc_angle_text_->SetEnabled(false);

        Reset();
    }
}

void RadialLayoutWidget::SetRadius(float radius) {
    radius_ = radius;
    UpdateRing_();
    UpdateSpokes_();
    UpdateAngles_();
}

void RadialLayoutWidget::SetAngles(const Anglef &start_angle,
                                   const Anglef &arc_angle) {
    start_angle_ = start_angle;
    arc_angle_   = arc_angle;
    start_spoke_->SetRotation(BuildRotation_(start_angle_));
    end_spoke_->SetRotation(BuildRotation_(arc_angle_));
    UpdateAngles_();
}

void RadialLayoutWidget::Reset() {
    radius_      = 1;
    start_angle_ = Anglef::FromDegrees(0);
    arc_angle_   = Anglef::FromDegrees(-360);

    UpdateRing_();
    UpdateSpokes_();
    UpdateAngles_();
}

void RadialLayoutWidget::RadiusActivated_(bool is_activation) {
    if (is_activation)
        start_radius_ = radius_;
    radius_text_->SetEnabled(is_activation);
    GetActivation().Notify(*this, is_activation);
}

void RadialLayoutWidget::RadiusChanged_(float change, float precision) {
    radius_ = RoundToPrecision(start_radius_ * change, precision);
    UpdateRing_();
    UpdateSpokes_();
    UpdateAngles_();
    changed_.Notify();
}

void RadialLayoutWidget::SpokeActivated_(bool is_activation, bool is_start) {
    if (is_activation)
        start_rot_angle_ = is_start ? start_angle_ : arc_angle_;
    start_angle_text_->SetEnabled(is_activation);
    arc_angle_text_->SetEnabled(is_activation);
    GetActivation().Notify(*this, is_activation);
}

void RadialLayoutWidget::SpokeChanged_(const Anglef &angle, bool is_start,
                                       float precision) {
    // Compute the new full angle and apply precision.
    Anglef new_angle = Anglef::FromDegrees(
        RoundToPrecision(
            NormalizedAngle(start_rot_angle_ + angle).Degrees(),
            precision));

    if (is_start) {
        if (new_angle.Degrees() == 360)
            new_angle = Anglef::FromDegrees(0);
        start_spoke_->SetRotationAngle(new_angle - start_rot_angle_);
        start_angle_ = new_angle;
    }
    else {
        if (new_angle.Degrees() == 0)
            new_angle = Anglef::FromDegrees(360);

        // Check for the end spoke crossing over the start spoke to change
        // direction.
        const float cur_d = arc_angle_.Degrees();
        const float new_d = new_angle.Degrees();
        // If switched to full 360 negative before but now slightly positive.
        if (cur_d == -360 && new_d < 180)
            new_angle = Anglef::FromDegrees(new_d);
        // Switching from positive to negative.
        else if (std::abs(cur_d - new_d) > 180 && new_d > cur_d)
            new_angle = Anglef::FromDegrees(new_d == 360 ? -360 : new_d - 360);

        arc_angle_ = new_angle;
        end_spoke_->SetRotationAngle(new_angle - start_rot_angle_);
    }
    UpdateAngles_();
    changed_.Notify();
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

    // Update the radius text.
    radius_text_->SetTranslation(
        Vector3f(.9f * radius_, kRadiusTextYOffset_, 0));
    radius_text_->SetText(Util::ToString(radius_));
}

void RadialLayoutWidget::UpdateSpokes_() {
    // If the radius is small, don't show the layout items (spokes and arc).
    const bool large_enough = radius_ >= kMinRadiusForSpokes_;
    layout_->SetEnabled(large_enough);

    // If spokes are visible, scale and translate them appropriately.
    if (large_enough) {
        const float spoke_size = kSpokeScale_ * radius_;
        Vector3f scale = spoke_geom_->GetScale();
        Vector3f trans = spoke_geom_->GetTranslation();
        scale[0] = spoke_size;
        trans[0] = .5f * spoke_size;
        spoke_geom_->SetScale(scale);
        spoke_geom_->SetTranslation(trans);
    }
}

void RadialLayoutWidget::UpdateAngles_() {
    // Update the arc line.
    auto &line = *SG::FindTypedShapeInNode<SG::PolyLine>(*arc_, "Line");
    line.SetArcPoints(start_angle_, arc_angle_, kArcRadiusScale_ * radius_,
                      kArcDegreesPerSegment_);

    // Update the angle text.
    auto get_text_pos = [&](const Anglef &angle, float y_off){
        const Point3f pos(.5f * radius_, y_off, 0);
        return BuildRotation_(angle) * pos;
    };
    start_angle_text_->SetTranslation(get_text_pos(start_angle_,
                                                   kStartAngleTextYOffset_));
    arc_angle_text_->SetTranslation(
        get_text_pos(start_angle_ + .5f * arc_angle_, kArcAngleTextYOffset_));
    start_angle_text_->SetText(GetAngleText_(start_angle_));
    arc_angle_text_->SetText(GetAngleText_(arc_angle_));
}

std::string RadialLayoutWidget::GetAngleText_(const Anglef &angle) {
    return Util::ToString(std::abs(angle.Degrees())) + kDegreeSign_;
}
