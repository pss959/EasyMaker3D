#include "Items/RadialMenu.h"

#include "App/ClickInfo.h"
#include "Items/RadialMenuInfo.h"
#include "Math/Polygon.h"
#include "SG/PolyLine.h"
#include "SG/Polygon.h"
#include "SG/Search.h"
#include "Util/Enum.h"
#include "Util/General.h"
#include "Widgets/PushButtonWidget.h"

namespace {

// Constants defining looks.
static const size_t kCirclePointCount_ = 72;
static const float  kOuterRadius_      = 4;
static const float  kInnerRadius_      = 1;
static const float  kMargin_           = .2f;
static const float  kInnerAngleMargin_ = 8;

// The outer angle margin is proportionally smaller to account for the larger
// radius.
static const float  kOuterAngleMargin_ =
    kInnerAngleMargin_ * (kInnerRadius_ / kOuterRadius_);

}  // anonymous namespace

void RadialMenu::CreationDone() {
    SG::Node::CreationDone();
    if (! IsTemplate()) {
        // Access the button template.
        button_ = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
        button_->SetEnabled(false);

        // And the parent node for the buttons.
        button_parent_ = SG::FindNodeUnderNode(*this, "Buttons");

        // Set up the border circle points.
        InitCircle_("Outer", kOuterRadius_);
        InitCircle_("Inner", kInnerRadius_);
    }
}

void RadialMenu::UpdateFromInfo(const RadialMenuInfo &info,
                                bool update_enabled) {
    // If the count changed, create and add a clone for each button.
    const size_t count = static_cast<size_t>(info.GetCount());
    if (buttons_.size() != count) {
        buttons_.resize(count);
        for (size_t i = 0; i < count; ++i)
            InitButton_(count, i, buttons_[i]);
        button_parent_->ClearChildren();
        for (size_t i = 0; i < count; ++i)
            button_parent_->AddChild(buttons_[i].widget);
    }
    for (size_t i = 0; i < count; ++i)
        ChangeButtonAction(i, info.GetButtonAction(i), update_enabled);
}

void RadialMenu::ChangeButtonAction(size_t index, Action action,
                                    bool update_enabled) {
    ASSERT(index < buttons_.size());
    const auto &but = buttons_[index];
    auto icon = SG::FindNodeUnderNode(*but.widget, "Icon");
    ASSERT(! icon->GetUniformBlocks().empty());
    auto icon_block = icon->GetUniformBlocks()[0];
    icon_block->SetSubImageName("MI" + Util::EnumToWord(action));

    // Hook up the button interaction.
    auto &clicked = but.widget->GetClicked();
    if (clicked.HasObserver(this))
        clicked.RemoveObserver(this);
    clicked.AddObserver(
        this, [this, index, action](const ClickInfo &){
            button_clicked_.Notify(index, action); });

    if (update_enabled)
        but.widget->SetInteractionEnabled(action != Action::kNone);
}

void RadialMenu::HighlightButton(const Anglef &angle) {
    // Get the Button_ containing the angle, if any. If it is enabled,
    // highlight it by hovering.
    for (auto &but: buttons_) {
        if (angle >= but.arc.start_angle &&
            angle <= but.arc.start_angle + but.arc.arc_angle) {
            if (but.widget != highlighted_button_) {
                if (highlighted_button_)
                    highlighted_button_->SetHovering(false);
                ClearHighlightedButton();
                if (but.widget->IsInteractionEnabled()) {
                    highlighted_button_ = but.widget;
                    but.widget->SetHovering(true);
                }
            }
            return;
        }
    }
}

void RadialMenu::ClearHighlightedButton() {
    highlighted_button_.reset();
}

void RadialMenu::SimulateButtonPress() {
    if (highlighted_button_) {
        ClickInfo info;
        info.widget = highlighted_button_.get();
        highlighted_button_->Click(info);
    }
}

void RadialMenu::InitCircle_(const std::string &name, float radius) {
    auto circles   = SG::FindNodeUnderNode(*this, "Circles");
    auto polyline  = SG::FindTypedShapeInNode<SG::PolyLine>(*circles, name);

    // Create a closed circle.
    std::vector<Point2f> circle_points =
        GetCirclePoints(kCirclePointCount_, 1, true);
    circle_points.push_back(circle_points[0]);

    polyline->SetPoints(
        Util::ConvertVector<Point3f, Point2f>(
            circle_points,
            [radius](const Point2f &p){
            return Point3f(radius * p[0], radius * p[1], 0); }));
}

void RadialMenu::InitButton_(size_t count, size_t index, Button_ &button) {
    // Create a clone of the button template.
    auto but = button_->CloneTyped<PushButtonWidget>(true);

    // Set up the geometry.
    Point2f center;
    std::vector<Point2f> points = GetButtonPoints_(count, index, center);
    auto area    = SG::FindNodeUnderNode(*but, "Area");
    auto border  = SG::FindNodeUnderNode(*but, "Border");
    auto icon    = SG::FindNodeUnderNode(*but, "Icon");
    auto line    = SG::FindTypedShapeInNode<SG::PolyLine>(*border, "Line");
    auto polygon = SG::FindTypedShapeInNode<SG::Polygon>(*area,    "Polygon");
    icon->SetTranslation(Point3f(center, .4f));
    polygon->SetPolygon(Polygon(points));

    // Close the loop, move the points slightly outward, and convert to 3D for
    // the border.
    std::vector<Point3f> border_points = Util::ConvertVector<Point3f, Point2f>(
        points, [](const Point2f &p){ return Point3f(1.01f * p, 0); });
    border_points.push_back(border_points[0]);
    line->SetPoints(border_points);

    // Enable the clone.
    but->SetEnabled(true);

    // Store the results.
    button.index  = index;
    button.arc    = ComputeArc_(count, index, 0);
    button.widget = but;
}

std::vector<Point2f> RadialMenu::GetButtonPoints_(size_t count, size_t index,
                                                  Point2f &center) {
    // Compute start and arc angles for inner and outer arcs, leaving the
    // correct margin around the angles to make button edges parallel.
    const CircleArc inner_arc = ComputeArc_(count, index, kInnerAngleMargin_);
    const CircleArc outer_arc = ComputeArc_(count, index, kOuterAngleMargin_);

    // Create the points for the wedge polygon and border.
    const size_t point_count = kCirclePointCount_ / count;
    const std::vector<Point2f> outer_points =
        GetCircleArcPoints(point_count, kOuterRadius_ - kMargin_, outer_arc);
    const std::vector<Point2f> inner_points =
        GetCircleArcPoints(point_count, kInnerRadius_ + kMargin_, inner_arc);

    // All outer points followed by all inner points reversed.
    std::vector<Point2f> points(2 * point_count);
    for (size_t i = 0; i < point_count; ++i) {
        points[i]                       = outer_points[i];
        points[2 * point_count - i - 1] = inner_points[i];
    }

    // The center is found by rotating the point halfway between the circles by
    // half of the arc angle.
    const Anglef half_angle = outer_arc.start_angle + .5f * outer_arc.arc_angle;
    const float  half_radius = .5f * (kInnerRadius_ + kOuterRadius_);
    center.Set(half_radius * ion::math::Cosine(half_angle),
               half_radius * ion::math::Sine(half_angle));

    return points;
}

CircleArc RadialMenu::ComputeArc_(size_t count, size_t index, float margin) {
    const Anglef margin_angle = Anglef::FromDegrees(margin);
    CircleArc arc;
    arc.arc_angle   = Anglef::FromDegrees(360) / count - margin_angle;
    arc.start_angle =
        index * (arc.arc_angle + margin_angle) + .5f * margin_angle;
    return arc;
}
