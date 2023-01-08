#include "Widgets/RadialLayoutWidget.h"

#include <algorithm>

#include "Base/Tuning.h"
#include "Math/Linear.h"
#include "SG/ColorMap.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "SG/Torus.h"
#include "SG/Tube.h"
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
        end_angle_text_   = get_text("EndAngleText");
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
        end_angle_text_->SetEnabled(false);
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
    end_angle_ = NormalizedAngle(arc_.start_angle + arc_.arc_angle);
    start_spoke_->SetRotationAngle(arc_.start_angle);
    end_spoke_->SetRotationAngle(end_angle_);
    UpdateArc_();
}

void RadialLayoutWidget::Reset() {
    radius_ = 1;
    arc_ = CircleArc(Anglef(), Anglef::FromDegrees(-360));
    end_angle_ = Anglef();

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
    const Color start  = get_color("RLWStartSpokeColor");
    const Color end    = get_color("RLWEndSpokeColor");
    const Color arc    = get_color("RLWArcColor");

    // Various geometry.
    start_spoke_->SetInactiveColor(start);
    start_spoke_->SetActiveColor(get_color("RLWStartSpokeActiveColor"));
    end_spoke_->SetInactiveColor(end);
    end_spoke_->SetActiveColor(get_color("RLWEndSpokeActiveColor"));
    arc_line_->SetBaseColor(arc);

    // Text.
    radius_text_->SetTextColor(active);
    start_angle_text_->SetTextColor(start);
    end_angle_text_->SetTextColor(end);
    arc_angle_text_->SetTextColor(arc);
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
        start_rot_angle_ = is_start ? arc_.start_angle : end_angle_;
    start_angle_text_->SetEnabled(is_activation);
    end_angle_text_->SetEnabled(is_activation);
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
    if (new_angle.Degrees() == 360)
        new_angle = Anglef::FromDegrees(0);

    // When the start spoke is moved, both spokes rotate together.
    if (is_start) {
        start_spoke_->SetRotationAngle(new_angle);
        arc_.start_angle = new_angle;
        end_angle_ = NormalizedAngle(arc_.start_angle + arc_.arc_angle);
        end_spoke_->SetRotationAngle(end_angle_);
    }
    // When the end spoke is moved, only it rotates.
    else {
        // Compute the new arc angle. Check first for the end spoke crossing
        // over the start spoke. This will return 0 if no crossover.
        Anglef new_arc_angle = GetCrossoverAngle_(new_angle);

        // If there was no crossover, keep going in the same direction.
        if (new_arc_angle.Radians() == 0) {
            const bool is_ccw = arc_.arc_angle.Radians() >= 0;
            new_arc_angle = NormalizedAngle(new_angle - arc_.start_angle) -
                Anglef::FromDegrees(is_ccw ? 0 : 360);
            // Don't allow 0 degrees.
            if (new_arc_angle.Radians() == 0)
                new_arc_angle = Anglef::FromDegrees(is_ccw ? 360 : -360);
        }
        arc_.arc_angle = new_arc_angle;
        end_angle_ = new_angle;
        end_spoke_->SetRotationAngle(end_angle_);
    }
    UpdateArc_();
    changed_.Notify();
}

Anglef RadialLayoutWidget::GetCrossoverAngle_(const Anglef &new_end_angle) {
    // Returns the smaller signed angle between two angles.
    const auto deg_diff = [](const Anglef &from_angle, const Anglef &to_angle){
        float deg = (to_angle - from_angle).Degrees();
        return deg > 180 ? deg - 360 : deg < -180 ? deg + 360 : deg;
    };

    // Determine the angle between the start and end spokes previously and now.
    const float prev_diff_deg = deg_diff(arc_.start_angle, end_angle_);
    const float  new_diff_deg = deg_diff(arc_.start_angle, new_end_angle);

    bool use_angle = false;

    // If there is no current arc, treat this as a crossover.
    if (arc_.arc_angle.Radians() == 0)
        use_angle = true;

    // Check for a sign change in the subtended angles. The sign can change
    // near 0 or near 180. Ignore changes near 180.
    else use_angle = (std::abs(new_diff_deg) < 90 &&
                      ((prev_diff_deg <= 0 && new_diff_deg > 0) ||
                       (prev_diff_deg >= 0 && new_diff_deg < 0)));

    return use_angle ? Anglef::FromDegrees(new_diff_deg) : Anglef();
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
    radius_text_->SetWorldScaleAndRotation(
        text_matrix_, TK::kLinearFeedbackTextScale, text_rotation_);
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
    auto &line = *SG::FindTypedShapeInNode<SG::Tube>(*arc_line_, "Line");
    line.SetArcPoints(arc_, TK::kRLWArcRadiusScale * radius_,
                      TK::kRLWArcDegreesPerSegment);

    // Text position helper.
    auto tpos = [&](const Anglef &angle, float radius_fraction, float y_off){
        const Point3f pos(radius_fraction * radius_, y_off, 0);
        return BuildRotation_(angle) * pos;
    };

    // Update the scale, rotation, translation, and text.
    auto update_angle = [&](SG::TextNode &text_node, const Anglef &angle,
                            const Point3f &pos) {
        text_node.SetWorldScaleAndRotation(text_matrix_, TK::kRLWTextScale,
                                           text_rotation_);
        text_node.SetTranslation(pos);
        text_node.SetText(GetAngleText_(angle));
    };

    const Anglef &sa = arc_.start_angle;
    const Anglef &aa = arc_.arc_angle;
    const Anglef  ea = end_spoke_->GetRotationAngle();

    const Point3f sa_pos = tpos(sa, TK::kRLWStartEndAngleTextRadiusScale,
                                TK::kRLWStartEndAngleTextYOffset);
    const Point3f ea_pos = tpos(ea, TK::kRLWStartEndAngleTextRadiusScale,
                                TK::kRLWStartEndAngleTextYOffset);
    const Point3f aa_pos = tpos(sa + .5f * aa, TK::kRLWArcRadiusScale,
                                TK::kRLWArcAngleTextYOffset);

    update_angle(*start_angle_text_, sa, sa_pos);
    update_angle(*end_angle_text_,   ea, ea_pos);
    update_angle(*arc_angle_text_,   aa, aa_pos);
}

std::string RadialLayoutWidget::GetAngleText_(const Anglef &angle) {
    return Util::ToString(std::abs(angle.Degrees())) + TK::kDegreeSign;
}
