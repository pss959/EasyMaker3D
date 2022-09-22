#include "Widgets/RadialLayoutWidget.h"

#include <algorithm>

#include "Base/Tuning.h"
#include "Math/Linear.h"
#include "SG/ColorMap.h"
#include "SG/PolyLine.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "SG/Torus.h"
#include "Widgets/DiscWidget.h"

#include <ion/math/angleutils.h>

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
        arc_line_         = SG::FindNodeUnderNode(*this, "Arc");
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

        // This sets the colors for the main widget parts.
        SetColorNamePrefix("Target");
        ring_->SetColorNamePrefix("Target");

        Reset();
    }
}

void RadialLayoutWidget::SetRadius(float radius) {
    radius_ = radius;
    UpdateRing_();
    UpdateSpokes_();
    UpdateArc_();
}

void RadialLayoutWidget::SetArc(const CircleArc &arc) {
    arc_ = arc;
    start_spoke_->SetRotation(BuildRotation_(arc.start_angle));
    end_spoke_->SetRotation(BuildRotation_(arc.arc_angle));
    UpdateArc_();
}

void RadialLayoutWidget::Reset() {
    radius_      = 1;
    arc_ = CircleArc(Anglef(), Anglef::FromDegrees(-360));

    UpdateRing_();
    UpdateSpokes_();
    UpdateArc_();
}

void RadialLayoutWidget::PostSetUpIon() {
    Widget::PostSetUpIon();
    SetColors_();
}

void RadialLayoutWidget::SetColors_() {
    auto get_color = [](const std::string &name){
        return SG::ColorMap::SGetColor(name);
    };

    const Color active = get_color("TargetActiveColor");
    const Color start  = get_color("RadialLayoutStartSpokeColor");
    const Color end    = get_color("RadialLayoutEndSpokeColor");
    const Color arc    = get_color("RadialLayoutArcColor");

    start_spoke_->SetInactiveColor(start);
    end_spoke_->SetInactiveColor(end);
    arc_line_->SetBaseColor(arc);
    radius_text_->SetBaseColor(active);
    start_angle_text_->SetBaseColor(start);
    arc_angle_text_->SetBaseColor(arc);
}

void RadialLayoutWidget::RadiusActivated_(bool is_activation) {
    if (is_activation)
        start_radius_ = radius_;
    radius_text_->SetEnabled(is_activation);
    GetActivation().Notify(*this, is_activation);
}

void RadialLayoutWidget::RadiusChanged_(float change, float precision) {
    radius_ = std::max(RoundToPrecision(start_radius_ * change, precision),
                       TK::kRLWRingMinOuterRadius);
    UpdateRing_();
    UpdateSpokes_();
    UpdateArc_();
    changed_.Notify();
}

void RadialLayoutWidget::SpokeActivated_(bool is_activation, bool is_start) {
    if (is_activation)
        start_rot_angle_ = is_start ? arc_.start_angle : arc_.arc_angle;
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
        arc_.start_angle = new_angle;
    }
    else {
        if (new_angle.Degrees() == 0)
            new_angle = Anglef::FromDegrees(360);

        // Check for the end spoke crossing over the start spoke to change
        // direction.
        const float cur_d = arc_.arc_angle.Degrees();
        const float new_d = new_angle.Degrees();
        // If switched to full 360 negative before but now slightly positive.
        if (cur_d == -360 && new_d < 180)
            new_angle = Anglef::FromDegrees(new_d);
        // Switching from positive to negative.
        else if (std::abs(cur_d - new_d) > 180 && new_d > cur_d)
            new_angle = Anglef::FromDegrees(new_d == 360 ? -360 : new_d - 360);

        arc_.arc_angle = new_angle;
        end_spoke_->SetRotationAngle(new_angle - start_rot_angle_);
    }
    UpdateArc_();
    changed_.Notify();
}

void RadialLayoutWidget::UpdateRing_() {
    // Compute a reasonable inner radius and number of sectors based on the
    // current radius.
    const float inner_radius = Clamp(
        .1f * radius_, TK::kRLWRingMinInnerRadius, TK::kRLWRingMaxInnerRadius);
    const int sector_count = Clamp(static_cast<int>(32 * radius_),
                                   TK::kRLWRingMinSectorCount,
                                   TK::kRLWRingMaxSectorCount);

    // Update the ring torus geometry.
    auto &torus = *SG::FindTypedShapeInNode<SG::Torus>(*ring_, "Torus");
    torus.SetGeometry(inner_radius, radius_,
                      TK::kRLWRingRingCount, sector_count);

    // Update the radius text.
    radius_text_->SetTranslation(Vector3f(.9f * radius_,
                                          TK::kRLWRadiusTextYOffset, 0));
    radius_text_->SetText(Util::ToString(radius_));
}

void RadialLayoutWidget::UpdateSpokes_() {
    // If the radius is small, don't show the layout items (spokes and arc).
    const bool large_enough = radius_ >= TK::kRLWMinRadiusForSpokes;
    layout_->SetEnabled(large_enough);

    // If spokes are visible, scale and translate them appropriately.
    if (large_enough) {
        const float spoke_size = TK::kRLWSpokeScale * radius_;
        Vector3f scale = spoke_geom_->GetScale();
        Vector3f trans = spoke_geom_->GetTranslation();
        scale[0] = spoke_size;
        trans[0] = .5f * spoke_size;
        spoke_geom_->SetScale(scale);
        spoke_geom_->SetTranslation(trans);
    }
}

void RadialLayoutWidget::UpdateArc_() {
    // Update the arc line.
    auto &line = *SG::FindTypedShapeInNode<SG::PolyLine>(*arc_line_, "Line");
    line.SetArcPoints(arc_, TK::kRLWArcRadiusScale * radius_,
                      TK::kRLWArcDegreesPerSegment);

    // Update the angle text.
    auto tpos = [&](const Anglef &angle, float y_off){
        const Point3f pos(.5f * radius_, y_off, 0);
        return BuildRotation_(angle) * pos;
    };
    const Anglef &sa = arc_.start_angle;
    const Anglef &aa = arc_.arc_angle;
    start_angle_text_->SetTranslation(tpos(sa, TK::kRLWStartAngleTextYOffset));
    arc_angle_text_->SetTranslation(tpos(sa + .5f * aa,
                                         TK::kRLWArcAngleTextYOffset));
    start_angle_text_->SetText(GetAngleText_(sa));
    arc_angle_text_->SetText(GetAngleText_(aa));
}

std::string RadialLayoutWidget::GetAngleText_(const Anglef &angle) {
    return Util::ToString(std::abs(angle.Degrees())) + TK::kDegreeSign;
}
