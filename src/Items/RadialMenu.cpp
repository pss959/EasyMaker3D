#include "Items/RadialMenu.h"

#include "Math/Curves.h"
#include "Math/Polygon.h"
#include "SG/PolyLine.h"
#include "SG/Polygon.h"
#include "SG/Search.h"
#include "Util/General.h"

namespace {

// Constants defining looks.
static const float  kOuterRadius_      = 4;
static const float  kInnerRadius_      = 1;
static const float  kMargin_           = .2f;
static const float  kAngleMargin_      = 2;
static const size_t kCirclePointCount_ = 72;

}  // anonymous namespace

void RadialMenu::CreationDone() {
    SG::Node::CreationDone();
    if (! IsTemplate()) {
        // Access the button template.
        button_ = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
        button_->SetEnabled(false);

        // And the parent node for the buttons.
        buttons_ = SG::FindNodeUnderNode(*this, "Buttons");

        // Set up the border circle points.
        InitCircle_("Outer", kOuterRadius_);
        InitCircle_("Inner", kInnerRadius_);
    }
}

void RadialMenu::UpdateFromInfo(const RadialMenuInfo &info) {
    buttons_->ClearChildren();

    // Create and add a clone for each button.
    const size_t count = static_cast<size_t>(info.GetCount());
    for (size_t i = 0; i < count; ++i)
        buttons_->AddChild(InitButton_(count, i, info.GetButtonAction(i)));
}

void RadialMenu::InitCircle_(const std::string &name, float radius) {
    auto circles   = SG::FindNodeUnderNode(*this, "Circles");
    auto polyline  = SG::FindTypedShapeInNode<SG::PolyLine>(*circles, name);

    // Create a closed circle.
    std::vector<Point2f> circle_points = GetCirclePoints(kCirclePointCount_, 1);
    circle_points.push_back(circle_points[0]);

    polyline->SetPoints(
        Util::ConvertVector<Point3f, Point2f>(
            circle_points,
            [radius](const Point2f &p){
            return Point3f(radius * p[0], radius * p[1], 0); }));
}

PushButtonWidgetPtr RadialMenu::InitButton_(size_t count, size_t index,
                                            Action action) {
    // Create a clone of the button template.
    auto but = button_->CloneTyped<PushButtonWidget>(true);

    // Set up the geometry.
    std::vector<Point2f> points = GetButtonPoints_(count, index);
    auto area    = SG::FindNodeUnderNode(*but, "Area");
    auto border  = SG::FindNodeUnderNode(*but, "Border");
    auto line    = SG::FindTypedShapeInNode<SG::PolyLine>(*border, "Line");
    auto polygon = SG::FindTypedShapeInNode<SG::Polygon>(*area,    "Polygon");
    polygon->SetPolygon(Polygon(points));

    // Close the loop, move the points slightly outward, and convert to 3D for
    // the border.
    std::vector<Point3f> border_points = Util::ConvertVector<Point3f, Point2f>(
        points, [](const Point2f &p){ return Point3f(1.01f * p, 0); });
    border_points.push_back(border_points[0]);
    line->SetPoints(border_points);

    // Hook up the button interaction.
    but->GetClicked().AddObserver(
        this, [this, index, action](const ClickInfo &){
            button_clicked_.Notify(index, action); });

    // Enable the clone.
    but->SetEnabled(true);

    return but;
}

std::vector<Point2f> RadialMenu::GetButtonPoints_(size_t count, size_t index) {
    Anglef outer_arc_angle   = Anglef::FromDegrees(360) / count;
    Anglef outer_start_angle = index * outer_arc_angle;
    Anglef inner_start_angle, inner_arc_angle;
    if (count == 1) {
        inner_start_angle = outer_start_angle;
        inner_arc_angle   = outer_arc_angle;
    }
    else {
        const Anglef margin = Anglef::FromDegrees(kAngleMargin_);
        outer_start_angle += margin;
        outer_arc_angle   -= 2 * margin;

        // Adjust inner angles to make button edges parallel.
        const float ratio = .5f * kOuterRadius_ / kInnerRadius_;
        inner_start_angle = outer_start_angle + ratio * margin;
        inner_arc_angle   = outer_arc_angle   - 2 * ratio * margin;
    }

    // Create the points for the wedge polygon and border.
    const size_t point_count = kCirclePointCount_ / count;
    const std::vector<Point2f> outer_points =
        GetCircleArcPoints(point_count, kOuterRadius_ - kMargin_,
                           outer_start_angle, outer_arc_angle);
    const std::vector<Point2f> inner_points =
        GetCircleArcPoints(point_count, kInnerRadius_ + kMargin_,
                           inner_start_angle, inner_arc_angle);

    // All outer points followed by all inner points reversed.
    std::vector<Point2f> points(2 * point_count);
    for (size_t i = 0; i < point_count; ++i) {
        points[i]                       = outer_points[i];
        points[2 * point_count - i - 1] = inner_points[i];
    }

    return points;
}
