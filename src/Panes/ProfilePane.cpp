#include "Panes/ProfilePane.h"

#include <functional>
#include <limits>
#include <string>
#include <vector>

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Math/Snap2D.h"
#include "Place/ClickInfo.h"
#include "SG/ColorMap.h"
#include "SG/Node.h"
#include "SG/PolyLine.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/String.h"
#include "Util/Tuning.h"
#include "Widgets/GenericWidget.h"
#include "Widgets/Slider2DWidget.h"

// ----------------------------------------------------------------------------
// ProfilePane::Impl_ class.
// ----------------------------------------------------------------------------

class ProfilePane::Impl_ {
  public:
    /// Typedef for function to set the range for a movable point
    /// Slider2DWidget.
    typedef std::function<Range2f(Slider2DWidget &,
                                  const Profile &, size_t)> SliderRangeFunc;

    /// Typedef for function that determines whether a new point can be
    /// inserted in the Profile at a given index.
    typedef std::function<bool(const Profile &, size_t)> CanInsertPointFunc;

    Impl_(SG::Node &root_node,
          const SliderRangeFunc &slider_range_func,
          const CanInsertPointFunc &can_insert_point_func);

    // Sets colors on certain parts. This has to be done after Ion is set up.
    void SetColors();

    Util::Notifier<bool> & GetActivation()     { return activation_;      }
    Util::Notifier<>     & GetProfileChanged() { return profile_changed_; }
    void SetProfile(const Profile &profile);
    const Profile & GetProfile() const { return profile_; }
    void SetPointPrecision(const Vector2f &xy_precision) {
        precision_ = xy_precision;
    }
    void AdjustSize(const Vector2f &base_size, const Vector2f &size);
    ClickableWidgetPtr GetGripWidget(const Point2f &p);
    WidgetPtr GetTouchedWidget(const TouchInfo &info, float &closest_distance);

  private:
    SG::Node           &root_node_;
    SliderRangeFunc    slider_range_func_;
    CanInsertPointFunc can_insert_point_func_;

    Vector2f           precision_{0, 0};
    Snap2D             snapper_;

    /// Profile being edited.
    Profile            profile_;

    /// Notifies when any interaction starts or ends.
    Util::Notifier<bool> activation_;

    /// Notifies when any interactive change is made to the Profile.
    Util::Notifier<>     profile_changed_;

    // Parts
    SG::NodePtr       start_point_;     ///< Fixed start point indicator.
    SG::NodePtr       end_point_;       ///< Fixed end point indicator.
    SG::NodePtr       movable_parent_;  ///< Parent of movable point widgets.
    Slider2DWidgetPtr movable_slider_;  ///< Slider cloned per movable point.
    WidgetPtr         delete_spot_;     ///< Target spot for deleting points.
    GenericWidgetPtr  area_widget_;     ///< Detects drags in Pane area.
    GenericWidgetPtr  new_point_;       ///< Shows location of new point.
    SG::PolyLinePtr   profile_line_;    ///< Line showing Profile.
    SG::NodePtr       snap_feedback_;   ///< Shows point and line when snapped.
    SG::NodePtr       snapped_point_;   ///< Shows point when snapped.
    SG::PolyLinePtr   snapped_line_;    ///< Line showing snapped segments.

    /// Rectangle representing the drag target area used for deleting points.
    Range2f           delete_rect_;

    /// When the Profile line is dragged to create a new point, this stores the
    /// Slider2DWidget that the rest of the drag is delegated to. It is null
    /// the rest of the time.
    Slider2DWidgetPtr delegate_slider_;

    /// If the Profile is open, this sets the positions of the fixed point
    /// markers. Otherwise, it hides them.
    void PositionFixedPoints_();

    /// Updates the number of Slider2DWidget instances to match the number of
    /// movable Profile points.
    void CreateMovablePoints_();

    /// This is invoked when the user hovers inside the AreaWidget. It shows
    /// and positions the new_point_ widget if the hover is close to a Profile
    /// line segment.
    void AreaHovered_(const Point3f &point);

    /// This is invoked when the new_point_ widget is clicked, inserting a new
    /// Profile point if the new_point_ widget is close to a Profile line
    /// segment.
    void NewPointClicked_(const ClickInfo &info);

    /// This is invoked when the new_point_ widget is dragged. It uses the
    /// delegate_slider_ to create and drag a new movable Profile point.
    void NewPointDragged_(const DragInfo *info, bool is_start);

    /// This is invoked when a movable Profile point widget is activated.
    void PointActivated_(size_t index, bool is_activation);

    /// This is invoked when a movable Profile point widget is dragged.
    void PointMoved_(size_t index, const Point2f &pos);

    /// Potentially snaps the (movable) Profile point with the given index from
    /// the given position based on its neighbors. Updates \p point and returns
    /// true if it is snapped to either neighbor.
    bool SnapPoint_(size_t index, Point2f &point);

    /// Updates the range for the Slider2DWidget for each movable point.
    void UpdateSliderRanges_();

    /// Updates the profile_line_ based on the current Profile. If \p
    /// update_sliders is true, this also updates the positions of the movable
    /// point sliders.
    void UpdateLine_(bool update_sliders);

    /// Inserts a movable Profile point at the given index and position and
    /// returns the new movable point widget created for it.
    Slider2DWidgetPtr CreateDelegateSlider_(size_t index, const Point2f &pos);

    /// Returns the movable point widget for the indexed Profile point. Returns
    /// a null pointer if the index is for a fixed Profile point.
    Slider2DWidgetPtr GetMovableSlider_(size_t index) const;

    /// If the given point is near a Profile line segment but not too close to
    /// any existing point, this sets \p start_index to the index of the
    /// starting Profile point of that segment and returns true. Otherwise, it
    /// just returns false.
    bool IsNearProfileSegment_(const Point2f &pt, size_t &start_index);

    /// Puts the delete rectangle in a reasonable spot that is not too close to
    /// any of the profile points.
    void PositionDeleteSpot_();

    /// Returns the index of the closest movable Profile point to \p p and sets
    /// \p dist to its distance.
    size_t GetClosestMovablePoint_(const Point2f &p, float &dist);

    /// Sets \p mid_pt to the closest Profile midpoint to \p p and sets \p dist
    /// to its distance from it.
    void GetClosestMidPoint_(const Point2f &p, Point2f &mid_pt, float &dist);

    /// Sets 3D points in an SG::PolyLine from a vector of profile points.
    void SetLinePoints_(const Profile::PointVec &points, SG::PolyLine &line,
                        float z_offset);

    /// Converts a 3D point from the object coordinates of the ProfilePane to
    /// 2D profile coordinates.
    static Point2f ToProfile_(const Point3f &p) {
        return ToProfile_(ToPoint2f(p));
    }
    static Point2f ToProfile_(const Point2f &p) {
        // Convert X and Y from (-.5,.5) to (0,1).
        return Point2f(.5f + p[0], .5f + p[1]);
    }

    /// Does the opposite of ToProfile_ in 2 dimensions.
    static Point2f FromProfile2_(const Point2f &p, float z = 0) {
        // Convert X and Y from (0,1) to (-.5,.5).
        return Point2f(p[0] - .5f, p[1] - .5f);
    }

    /// Does the opposite of ToProfile_ in 3 dimensions. The Z coordinate is 0
    /// unless specified.
    static Point3f FromProfile3_(const Point2f &p, float z = 0) {
        return Point3f(FromProfile2_(p), z);
    }
};

ProfilePane::Impl_::Impl_(SG::Node &root_node,
                          const SliderRangeFunc &slider_range_func,
                          const CanInsertPointFunc &can_insert_point_func) :
    root_node_(root_node),
    slider_range_func_(slider_range_func),
    can_insert_point_func_(can_insert_point_func) {
    ASSERT(slider_range_func);
    ASSERT(can_insert_point_func);

    snapper_.SetToleranceAngle(TK::kProfilePaneMaxSnapAngle);

    // Find all the parts.
    start_point_    = SG::FindNodeUnderNode(root_node, "StartPoint");
    end_point_      = SG::FindNodeUnderNode(root_node, "EndPoint");
    movable_parent_ = SG::FindNodeUnderNode(root_node, "MovableParent");
    movable_slider_ = SG::FindTypedNodeUnderNode<Slider2DWidget>(
        root_node, "MovableSlider");
    delete_spot_ = SG::FindTypedNodeUnderNode<Widget>(root_node, "DeleteSpot");
    area_widget_ = SG::FindTypedNodeUnderNode<GenericWidget>(
        root_node, "AreaWidget");
    new_point_   = SG::FindTypedNodeUnderNode<GenericWidget>(
        root_node, "NewPoint");
    snap_feedback_ = SG::FindNodeUnderNode(root_node, "SnapFeedback");
    snapped_point_ = SG::FindNodeUnderNode(*snap_feedback_, "SnappedPoint");

    auto pline    = SG::FindNodeUnderNode(root_node,       "ProfileLine");
    auto sline    = SG::FindNodeUnderNode(*snap_feedback_, "SnappedLine");
    profile_line_ = SG::FindTypedShapeInNode<SG::PolyLine>(*pline, "Line");
    snapped_line_ = SG::FindTypedShapeInNode<SG::PolyLine>(*sline, "Line");

    area_widget_->GetHovered().AddObserver(
        this, [&](const Point3f &point){ AreaHovered_(point); });

    new_point_->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){ NewPointClicked_(info); });
    new_point_->GetDragged().AddObserver(
        this, [&](const DragInfo *info, bool is_start){
            NewPointDragged_(info, is_start); });

    // Initialize the size of the delete rectangle.
    const Vector3f size = delete_spot_->GetBounds().GetSize();
    delete_rect_ = BuildRange(Point2f::Zero(), Vector2f(size[0], size[1]));

    // Start with a valid Fixed Profile.
    SetProfile(ProfilePane::CreateDefaultProfile());
}

void ProfilePane::Impl_::SetColors() {
    const Color snapped_color = SG::ColorMap::SGetColor("ProfileSnapColor");
    snap_feedback_->SetBaseColor(snapped_color);
}

void ProfilePane::Impl_::SetProfile(const Profile &profile) {
    ASSERT(profile.IsValid());
    profile_ = profile;

    PositionFixedPoints_();
    CreateMovablePoints_();
    UpdateLine_(true);
}

void ProfilePane::Impl_::AdjustSize(const Vector2f &base_size,
                                    const Vector2f &size) {
    // Compute the scale that compensates for non-uniform aspect ratio as well
    // as changes in size, where the base_size uses a scale of 1.
    Vector3f scale(base_size[0] / size[0], base_size[1] / size[1], 1);

    // Adjust the fixed points, all movable sliders, and the delete spot to
    // compensate for nonuniform scale caused by the change in aspect ratio.
    start_point_->SetScale(scale);
    end_point_->SetScale(scale);
    for (const auto &ms: movable_parent_->GetChildren())
        ms->SetScale(scale);
    delete_spot_->SetScale(scale);
}

ClickableWidgetPtr ProfilePane::Impl_::GetGripWidget(const Point2f &p) {
    ClickableWidgetPtr widget;

    // Find the closest movable point, if there are any.
    int closest_pt = -1;
    float closest_pt_dist;
    if (profile_.GetMovablePointCount() > 0U)
        closest_pt = GetClosestMovablePoint_(p, closest_pt_dist);

    // Find the closest midpoint as well.
    float   closest_mid_dist;
    Point2f mid_pt;
    GetClosestMidPoint_(p, mid_pt, closest_mid_dist);

    if (closest_pt < 0 || closest_mid_dist < closest_pt_dist) {
        new_point_->TranslateTo(FromProfile3_(mid_pt, TK::kPaneZOffset));
        new_point_->SetEnabled(true);
        widget = new_point_;
    }
    else {
        new_point_->SetEnabled(false);
        widget = GetMovableSlider_(closest_pt);
    }

    return widget;
}

WidgetPtr ProfilePane::Impl_::GetTouchedWidget(const TouchInfo &info,
                                               float &closest_distance) {
    const auto &points = profile_.GetPoints();

    // Test movable points.
    WidgetPtr intersected_widget;
    for (size_t i = 0; i < points.size(); ++i) {
        if (auto slider = GetMovableSlider_(i)) {
            float dist;
            if (slider->IsTouched(info, dist) && dist < closest_distance) {
                closest_distance = dist;
                intersected_widget = slider;
            }
        }
    }

    // If no hit, try midpoints (unless already showing for a grip drag).
    if (! intersected_widget && ! new_point_->IsEnabled()) {
        new_point_->SetEnabled(true);
        for (size_t i = 1; i < points.size(); ++i) {
            const Point2f mp = .5f * (points[i - 1] + points[i]);
            new_point_->TranslateTo(FromProfile3_(mp, TK::kPaneZOffset));
            float dist;
            if (new_point_->IsTouched(info, dist) && dist < closest_distance) {
                closest_distance = dist;
                new_point_->SetEnabled(true);
                intersected_widget = new_point_;
                break;
            }
        }
        if (! intersected_widget)
            new_point_->SetEnabled(false);
    }

    return intersected_widget;
}

void ProfilePane::Impl_::PositionFixedPoints_() {
    const bool is_fixed = profile_.GetType() == Profile::Type::kFixed;
    if (is_fixed) {
        const auto &pts = profile_.GetPoints();
        start_point_->TranslateTo(FromProfile3_(pts.front()));
        end_point_->TranslateTo(FromProfile3_(pts.back()));
    }
    start_point_->SetEnabled(is_fixed);
    end_point_->SetEnabled(is_fixed);
}

void ProfilePane::Impl_::CreateMovablePoints_() {
    // Update the number of Slider2DWidget instances to match the number of
    // movable Profile points. The positions are set in UpdateLine_().

    const size_t num_needed  = profile_.GetMovablePointCount();
    const size_t num_present = movable_parent_->GetChildCount();
    if (num_present > num_needed) {
        while (movable_parent_->GetChildCount() > num_needed)
            movable_parent_->RemoveChild(movable_parent_->GetChildCount() - 1);
    }
    else {
        const bool is_fixed = profile_.GetType() == Profile::Type::kFixed;
        for (size_t i = num_present; i < num_needed; ++i) {
            // Get the index of the point in the Profile.
            const size_t index = i + (is_fixed ? 1 : 0);

            // No need for a deep clone for these.
            const Str name = "MovablePoint_" + Util::ToString(index);
            auto slider =
                movable_slider_->CloneTyped<Slider2DWidget>(false, name);
            slider->SetScale(start_point_->GetScale());
            slider->SetGripDragScale(TK::kPaneGripDragScale);

            slider->GetActivation().AddObserver(
                this, [&, index](Widget &, bool is_activation){
                    PointActivated_(index, is_activation); });
            slider->GetValueChanged().AddObserver(
                this, [&, index](Widget &, const Vector2f &v){
                    PointMoved_(index, ToProfile_(Point2f(v))); });
            slider->GetValueChanged().EnableObserver(this, false);
            movable_parent_->AddChild(slider);
            slider->SetEnabled(true);
        }

        // Update the range for all movable points.
        UpdateSliderRanges_();
    }
}

void ProfilePane::Impl_::AreaHovered_(const Point3f &point) {
    // If there is a 2D position and it is close enough to start a drag on a
    // line segment AND a new point can be created between the two neighbors,
    // show the new_point_ widget at the position.
    const Point2f pt = ToProfile_(point);
    size_t start_index;
    if (IsNearProfileSegment_(pt, start_index) &&
        can_insert_point_func_(profile_, start_index)) {
        new_point_->TranslateTo(FromProfile3_(pt, TK::kPaneZOffset));
        new_point_->SetEnabled(true);
    }
    else {
        new_point_->SetEnabled(false);
    }
}

void ProfilePane::Impl_::NewPointClicked_(const ClickInfo &info) {
    new_point_->SetEnabled(false);
    const Point2f pt = ToProfile_(Point3f(new_point_->GetTranslation()));
    size_t start_index;
    if (IsNearProfileSegment_(pt, start_index)) {
        profile_.InsertPoint(start_index + 1, pt);
        CreateMovablePoints_();
        UpdateLine_(true);
        profile_changed_.Notify();
    }
}

void ProfilePane::Impl_::NewPointDragged_(const DragInfo *info, bool is_start) {
    // Note that is_start is true for the start of a drag and info is null for
    // the end of a drag.
    if (is_start) {
        ASSERT(info);
        const Point2f pp = ToProfile_(Point3f(new_point_->GetTranslation()));
        size_t start_index;
        if (IsNearProfileSegment_(pp, start_index)) {
            new_point_->SetEnabled(false);
            delegate_slider_ = CreateDelegateSlider_(start_index + 1, pp);
            delegate_slider_->StartDrag(*info);
        }
    }
    else if (info) {  // Continued drag.
        if (delegate_slider_)
            delegate_slider_->ContinueDrag(*info);
    }
    else {            // End drag.
        if (delegate_slider_) {
            delegate_slider_->EndDrag();
            delegate_slider_.reset();
        }
    }
}

void ProfilePane::Impl_::PointActivated_(size_t index, bool is_activation) {
    auto slider = GetMovableSlider_(index);
    ASSERT(slider);

    if (is_activation) {
        // Detect point motion.
        slider->GetValueChanged().EnableObserver(this, true);

        // Put the delete spot in a good location unless this is a new point or
        // the minimum would be violated.
        if (! delegate_slider_ &&
            profile_.GetPointCount() > profile_.GetMinPointCount()) {
            PositionDeleteSpot_();
            delete_spot_->SetEnabled(true);
        }

        new_point_->SetEnabled(false);
    }
    else {
        // Stop tracking point motion.
        slider->GetValueChanged().EnableObserver(this, false);

        // If the point was dragged over the delete spot, delete it.
        if (delete_spot_->IsEnabled() &&
            delete_rect_.ContainsPoint(profile_.GetPoints()[index])) {
            profile_.RemovePoint(index);
            CreateMovablePoints_();
            UpdateLine_(true);
            profile_changed_.Notify();
        }
        else {
            // Update the ranges for all movable point sliders.
            UpdateSliderRanges_();
            // Make sure all movable points are in the correct place.
            UpdateLine_(true);
        }
        delete_spot_->SetEnabled(false);
        snap_feedback_->SetEnabled(false);
    }
    activation_.Notify(is_activation);
}

void ProfilePane::Impl_::PointMoved_(size_t index, const Point2f &pos) {
    // Index must be for a movable point.
    ASSERT(! profile_.IsFixedPoint(index));

    // Round to precision if requested.
    Point2f snapped_pos = pos;
    if (precision_[0] > 0)
        snapped_pos[0] = RoundToPrecision(snapped_pos[0], precision_[0]);
    if (precision_[1] > 0)
        snapped_pos[1] = RoundToPrecision(snapped_pos[1], precision_[1]);

    // Determine if modified-dragging is happening. If so, check for
    // horizontal, vertical, and 45-degree diagonal snapping.
    const bool should_snap =
        GetMovableSlider_(index)->GetCurrentDragInfo().is_modified_mode;
    snap_feedback_->SetEnabled(should_snap && SnapPoint_(index, snapped_pos));
    snapped_point_->TranslateTo(FromProfile3_(snapped_pos, 0));

    // Update the point in the Profile.
    profile_.SetPoint(index, snapped_pos);
    UpdateLine_(false);
    profile_changed_.Notify();

    // Highlight the delete spot if the point is over it.
    if (delete_spot_->IsEnabled())
        delete_spot_->SetActive(delete_rect_.ContainsPoint(snapped_pos));
}

bool ProfilePane::Impl_::SnapPoint_(size_t index, Point2f &point) {
    const Point2f &prev_pos = profile_.GetPreviousPoint(index);
    const Point2f &next_pos = profile_.GetNextPoint(index);

    // Compute the snapped point position and set the feedback line points.
    Profile::PointVec line_points;

    switch (snapper_.SnapPointBetween(prev_pos, next_pos, point)) {
      case Snap2D::Result::kNeither:
        return false;
      case Snap2D::Result::kPoint0:
        line_points = Profile::PointVec{ prev_pos, point };
        break;
      case Snap2D::Result::kPoint1:
        line_points = Profile::PointVec{ point, next_pos };
        break;
      case Snap2D::Result::kBoth:
        line_points = Profile::PointVec{ prev_pos, point, next_pos };
        break;
    }
    SetLinePoints_(line_points, *snapped_line_, 1.1 * TK::kPaneZOffset);

    return true;
}

void ProfilePane::Impl_::UpdateSliderRanges_() {
    for (size_t i = 0; i < profile_.GetPointCount(); ++i) {
        if (! profile_.IsFixedPoint(i)) {
            auto slider = GetMovableSlider_(i);
            const Range2f range = slider_range_func_(*slider, profile_, i);
            slider->SetRange(Vector2f(FromProfile2_(range.GetMinPoint())),
                             Vector2f(FromProfile2_(range.GetMaxPoint())));
        }
    }
}

void ProfilePane::Impl_::UpdateLine_(bool update_sliders) {
    // Update the line to connect all points.
    Profile::PointVec points = profile_.GetPoints();
    // Close the loop if the Profile is closed.
    if (profile_.GetType() == Profile::Type::kClosed)
        points.push_back(points[0]);
    SetLinePoints_(points, *profile_line_, TK::kPaneZOffset);

    // If requested, position the movable point sliders based on the points.
    if (update_sliders) {
        const auto &line_pts = profile_line_->GetPoints();
        const size_t point_count = profile_.GetPointCount();
        for (size_t index = 0; index < point_count; ++index) {
            if (auto slider = GetMovableSlider_(index))
                slider->SetValue(ToVector2f(Vector3f(line_pts[index])));
        }
    }
}

Slider2DWidgetPtr ProfilePane::Impl_::CreateDelegateSlider_(
    size_t index, const Point2f &pos) {
    profile_.InsertPoint(index, pos);
    CreateMovablePoints_();
    UpdateLine_(true);

    return GetMovableSlider_(index);
}

Slider2DWidgetPtr ProfilePane::Impl_::GetMovableSlider_(size_t index) const {
    Slider2DWidgetPtr slider;
    if (! profile_.IsFixedPoint(index)) {
        // Get the child index of the slider.
        const bool is_fixed = profile_.GetType() == Profile::Type::kFixed;
        const size_t child_index = index - (is_fixed ? 1 : 0);
        ASSERT(child_index < movable_parent_->GetChildCount());
        slider = std::dynamic_pointer_cast<Slider2DWidget>(
            movable_parent_->GetChild(child_index));
        ASSERT(slider);
    }
    return slider;
}

bool ProfilePane::Impl_::IsNearProfileSegment_(const Point2f &pt,
                                               size_t &start_index) {
    using ion::math::Distance;

    const auto &points = profile_.GetPoints();
    bool is_near_segment = false;
    for (size_t i = 0; i < points.size(); ++i) {
        // If the new point is too close to an existing profile point, stop.
        if (Distance(pt, points[i]) <= TK::kProfilePanePointTolerance)
            return false;

        // See if it is close enough to a segment.
        if (i > 0 && IsNearLineSegment(pt, points[i - 1], points[i],
                                       TK::kProfilePanePointTolerance)) {
            start_index = i - 1;
            is_near_segment = true;
        }
    }

    // If the Profile is closed, also test the closing segment.
    if (profile_.GetType() == Profile::Type::kClosed &&
        IsNearLineSegment(pt, points.back(), points[0],
                          TK::kProfilePanePointTolerance)) {
        start_index = points.size() - 1;
        is_near_segment = true;
    }

    return is_near_segment;
}

void ProfilePane::Impl_::PositionDeleteSpot_() {
    // Use one of the points in the center of one of the sides, a quarter way
    // in. Choose the one that is farthest from all movable Profile points.
    const Point2f cpts[4]{
        Point2f(.25f, .50f),  // Left edge.
        Point2f(.75f, .50f),  // Right edge.
        Point2f(.50f, .25f),  // Bottom edge.
        Point2f(.50f, .75f),  // Top edge.
    };

    float max_dist  = 0;
    int   max_index = -1;
    for (int i = 0; i < 4; ++i) {
        float dist;
        GetClosestMovablePoint_(cpts[i], dist);
        if (dist > max_dist) {
            max_index = i;
            max_dist  = dist;
        }
    }
    ASSERT(max_index >= 0 && max_index <= 3);
    const Point2f &pt = cpts[max_index];

    // Position the delete rectangle and the delete spot widget.
    delete_rect_ = BuildRange(pt, delete_rect_.GetSize());
    delete_spot_->TranslateTo(FromProfile3_(pt, 0));
}

size_t ProfilePane::Impl_::GetClosestMovablePoint_(const Point2f &p,
                                                   float &dist) {
    const auto &points = profile_.GetPoints();
    size_t closest = 0;
    dist = std::numeric_limits<float>::max();
    for (size_t i = 0; i < points.size(); ++i) {
        if (! profile_.IsFixedPoint(i)) {
            const float d = ion::math::Distance(p, points[i]);
            if (d < dist) {
                closest = i;
                dist    = d;
            }
        }
    }
    return closest;
}

void ProfilePane::Impl_::GetClosestMidPoint_(const Point2f &p,
                                             Point2f &mid_pt, float &dist) {
    const auto &points = profile_.GetPoints();
    dist = std::numeric_limits<float>::max();
    for (size_t i = 1; i < points.size(); ++i) {
        const Point2f mp = .5f * (points[i - 1] + points[i]);
        const float d = ion::math::Distance(p, mp);
        if (d < dist) {
            mid_pt = mp;
            dist    = d;
        }
    }
}

void ProfilePane::Impl_::SetLinePoints_(const Profile::PointVec &points,
                                        SG::PolyLine &line, float z_offset) {
    // Convert all points to 3D positions.
    auto convert_pt = [&](const Point2f &p){
        return FromProfile3_(p, z_offset);
    };
    line.SetPoints(Util::ConvertVector<Point3f, Point2f>(points, convert_pt));
}

// ----------------------------------------------------------------------------
// ProfilePane functions.
// ----------------------------------------------------------------------------

ProfilePane::ProfilePane() {
}

ProfilePane::~ProfilePane() {
}

void ProfilePane::CreationDone() {
    LeafPane::CreationDone();
    if (! IsTemplate()) {
        const auto srfunc = [&](Slider2DWidget &s, const Profile &p, size_t i){
            return GetMovablePointRange(s, p, i);
        };
        const auto cifunc = [&](const Profile &p, size_t i){
            return CanInsertPoint(p, i);
        };
        impl_.reset(new Impl_(*this, srfunc, cifunc));
    }
}

Util::Notifier<bool> & ProfilePane::GetActivation() {
    return impl_->GetActivation();
}

Util::Notifier<> & ProfilePane::GetProfileChanged() {
    return impl_->GetProfileChanged();
}

void ProfilePane::SetProfile(const Profile &profile) {
    impl_->SetProfile(profile);
}

const Profile & ProfilePane::GetProfile() const {
    return impl_->GetProfile();
}

Profile ProfilePane::CreateDefaultProfile() {
    return Profile::CreateFixedProfile(Point2f(0, 1), Point2f(1, 0), 2,
                                       Profile::PointVec());
}

void ProfilePane::SetPointPrecision(const Vector2f &xy_precision) {
    impl_->SetPointPrecision(xy_precision);
}

void ProfilePane::UpdateForLayoutSize(const Vector2f &size) {
    LeafPane::UpdateForLayoutSize(size);
    impl_->AdjustSize(GetBaseSize(), size);
}

ClickableWidgetPtr ProfilePane::GetGripWidget(const Point2f &p) {
    return impl_->GetGripWidget(p);
}

WidgetPtr ProfilePane::GetTouchedWidget(const TouchInfo &info,
                                        float &closest_distance) {
    return impl_->GetTouchedWidget(info, closest_distance);
}

IPaneInteractor * ProfilePane::GetInteractor() {
    return this;
}

BorderPtr ProfilePane::GetFocusBorder() const {
    // Cannot take focus - would not make sense.
    return BorderPtr();
}

void ProfilePane::PostSetUpIon() {
    LeafPane::PostSetUpIon();
    impl_->SetColors();
}

Range2f ProfilePane::GetMovablePointRange(Slider2DWidget &slider,
                                          const Profile &profile,
                                          size_t index) const {
    return Range2f(Point2f(0, 0), Point2f(1, 1));
}

bool ProfilePane::CanInsertPoint(const Profile &profile, size_t index) const {
    return true;
}
