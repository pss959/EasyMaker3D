#include "Panes/ProfilePane.h"

#include <limits>
#include <string>
#include <vector>

#include <ion/math/vectorutils.h>

#include "App/ClickInfo.h"
#include "Base/Tuning.h"
#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/PolyLine.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/String.h"
#include "Widgets/GenericWidget.h"
#include "Widgets/Slider2DWidget.h"

// ----------------------------------------------------------------------------
// ProfilePane::Impl_ class.
// ----------------------------------------------------------------------------

class ProfilePane::Impl_ {
  public:
    Impl_(SG::Node &root_node, size_t min_point_count);

    Util::Notifier<bool> & GetActivation() { return activation_; }
    Util::Notifier<const Profile &> & GetProfileChanged() {
        return profile_changed_;
    }
    void SetProfile(const Profile &profile);
    const Profile & GetProfile() const { return profile_; }
    void AdjustSize(const Vector2f &base_size, const Vector2f &size);
    ClickableWidgetPtr GetGripWidget(const Point2f &p);

  private:
    SG::Node     &root_node_;
    const size_t  min_point_count_;

    /// Profile being edited.
    Profile profile_;

    /// Notifies when any interaction starts or ends.
    Util::Notifier<bool> activation_;

    /// Notifies when any interactive change is made to the Profile.
    Util::Notifier<const Profile &> profile_changed_;

    // Parts
    SG::NodePtr       start_point_;     ///< Fixed start point indicator.
    SG::NodePtr       end_point_;       ///< Fixed end point indicator.
    SG::NodePtr       movable_parent_;  ///< Parent of movable point widgets.
    Slider2DWidgetPtr movable_slider_;  ///< Slider cloned per movable point.
    WidgetPtr         delete_spot_;     ///< Target spot for deleting points.
    GenericWidgetPtr  area_widget_;     ///< Detects drags in Pane area.
    GenericWidgetPtr  new_point_;       ///< Shows location of new point.
    SG::PolyLinePtr   profile_line_;    ///< Line showing Profile.

    /// Rectangle representing the drag target area used for deleting points.
    Range2f           delete_rect_;

    /// When the Profile line is dragged to create a new point, this stores the
    /// Slider2DWidget that the rest of the drag is delegated to. It is null
    /// the rest of the time.
    Slider2DWidgetPtr delegate_slider_;

    void PositionFixedPoints_();
    void CreateMovablePoints_();
    void PositionDeleteRect_(const Point2f &pos);
    void AreaHovered_(const Point3f &point);
    void AreaDragged_(const DragInfo *info, bool is_start);
    void NewPointClicked_(const ClickInfo &info);
    void NewPointDragged_(const DragInfo *info, bool is_start);
    void PointActivated_(size_t index, bool is_activation);
    void PointMoved_(size_t index, const Point2f &pos);
    void UpdateLine_(bool update_points);
    void CreateDelegateSlider_(size_t index, const Point2f &pos);
    Slider2DWidgetPtr GetMovableSlider_(size_t index) const;

    /// If the given point is on the profile line but not too close to any
    /// existing point, this returns the index (> 0) into the full set of
    /// profile points (including the fixed points at the ends) of where to
    /// create a new point. Otherwise, it returns -1.
    int GetNewPointIndex_(const Point2f &pt);

    static bool IsPointOnSegment_(const Point2f &p,
                                  const Point2f &end0, const Point2f &end1);

    /// Puts the delete rectangle in a reasonable spot that is not too close to
    /// any of the profile points.
    void PositionDeleteSpot_();

    /// Returns the index of the closest point in the given vector to p and
    /// sets dist to its distance.
    size_t GetClosestPoint_(const std::vector<Point2f> &points,
                            const Point2f &p, float &dist);

    /// Sets mid_pt to the coordinates of the closest midpoint to p and sets
    /// dist to its distance from it.
    void GetClosestMidPoint_(const std::vector<Point2f> &points,
                             const Point2f &p, Point2f &mid_pt, float &dist);

    /// Converts a 3D point from the object coordinates of the ProfilePane to
    /// 2D profile coordinates.
    static Point2f ToProfile_(const Point3f &p) {
        return ToProfile_(ToPoint2f(p));
    }
    static Point2f ToProfile_(const Point2f &p) {
        // Convert X and Y from (-.5,.5) to (0,1).
        return Point2f(.5f + p[0], .5f + p[1]);
    }

    /// Does the opposite of ToProfile_. The Z coordinate is 0 unless specified.
    static Point3f FromProfile_(const Point2f &p, float z = 0) {
        // Convert X and Y from (0,1) to (-.5,.5).
        return Point3f(p[0] - .5f, p[1] - .5f, z);
    }
};

ProfilePane::Impl_::Impl_(SG::Node &root_node, size_t min_point_count) :
    root_node_(root_node),
    min_point_count_(min_point_count) {

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

    auto line = SG::FindNodeUnderNode(root_node, "ProfileLine");
    profile_line_ = SG::FindTypedShapeInNode<SG::PolyLine>(*line, "Line");

    area_widget_->GetHovered().AddObserver(
        this, [&](const Point3f &point){ AreaHovered_(point); });
    area_widget_->GetDragged().AddObserver(
        this, [&](const DragInfo *info, bool is_start){
            AreaDragged_(info, is_start); });

    new_point_->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){ NewPointClicked_(info); });
    new_point_->GetDragged().AddObserver(
        this, [&](const DragInfo *info, bool is_start){
            NewPointDragged_(info, is_start); });

    // Initialize the size of the delete rectangle.
    const Vector3f size = delete_spot_->GetBounds().GetSize();
    delete_rect_ = BuildRange(Point2f::Zero(), Vector2f(size[0], size[1]));

    PositionFixedPoints_();
    CreateMovablePoints_();
    UpdateLine_(true);
}

void ProfilePane::Impl_::SetProfile(const Profile &profile) {
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
    if (! profile_.GetPoints().empty())
        closest_pt = GetClosestPoint_(profile_.GetPoints(), p, closest_pt_dist);

    // Find the closest midpoint as well.
    float closest_mid_dist;
    Point2f mid_pt;
    GetClosestMidPoint_(profile_.GetAllPoints(), p, mid_pt, closest_mid_dist);

    if (closest_pt < 0 || closest_mid_dist < closest_pt_dist) {
        new_point_->SetTranslation(FromProfile_(mid_pt, TK::kPaneZOffset));
        new_point_->SetEnabled(true);
        widget = new_point_;
    }
    else {
        new_point_->SetEnabled(false);
        widget = GetMovableSlider_(closest_pt);
    }

    return widget;
}

void ProfilePane::Impl_::PositionFixedPoints_() {
    start_point_->SetTranslation(FromProfile_(profile_.GetStartPoint()));
    end_point_->SetTranslation(FromProfile_(profile_.GetEndPoint()));
}

void ProfilePane::Impl_::CreateMovablePoints_() {
    // Update the number of Slider2DWidget instances to match the number of
    // interior Profile points. The positions are set in UpdateLine_().

    const size_t num_needed  = profile_.GetPoints().size();
    const size_t num_present = movable_parent_->GetChildCount();
    if (num_present > num_needed) {
        while (movable_parent_->GetChildCount() > num_needed)
            movable_parent_->RemoveChild(movable_parent_->GetChildCount() - 1);
    }
    else {
        for (size_t index = num_present; index < num_needed; ++index) {
            // No need for a deep clone for these.
            const std::string name = "MovablePoint_" + Util::ToString(index);
            auto slider =
                movable_slider_->CloneTyped<Slider2DWidget>(false, name);
            slider->SetScale(start_point_->GetScale());
            slider->SetGripDragScale(TK::kPaneGripDragScale);

            // Use the same range as Pane coordinates: (-.5,.5).
            slider->SetRange(Vector2f(-.5f, -.5f), Vector2f(.5f, .5f));
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
    }
}

void ProfilePane::Impl_::PositionDeleteRect_(const Point2f &pos) {
    delete_rect_ = BuildRange(pos, delete_rect_.GetSize());
}

void ProfilePane::Impl_::AreaHovered_(const Point3f &point) {
    // If there is a 2D position and it is close enough to start a drag on a
    // line segment, show the midpoint at the position.
    const Point2f pos = ToProfile_(point);
    int index = GetNewPointIndex_(pos);
    if (index >= 0) {
        new_point_->SetTranslation(FromProfile_(pos, TK::kPaneZOffset));
        new_point_->SetEnabled(true);
    }
    else {
        new_point_->SetEnabled(false);
    }
}

void ProfilePane::Impl_::AreaDragged_(const DragInfo *info, bool is_start) {
    // Handle only touch interaction.
    if (! info || info->trigger != Trigger::kTouch)
        return;

    // Note that is_start is true for the start of a drag and info is null for
    // the end of a drag.
    if (is_start) {
        ASSERT(info);
        const CoordConv cc(info->path_to_widget);
        const Point2f pp = ToProfile_(cc.RootToObject(info->touch_position));
        int index = GetNewPointIndex_(pp);
        if (index > 0) {
            new_point_->SetEnabled(false);
            CreateDelegateSlider_(index - 1, pp);
            delegate_slider_ = GetMovableSlider_(index - 1);
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

void ProfilePane::Impl_::NewPointClicked_(const ClickInfo &info) {
    new_point_->SetEnabled(false);
    const Point2f pt = ToProfile_(Point3f(new_point_->GetTranslation()));
    const int index = GetNewPointIndex_(pt);
    if (index > 0) {
        profile_.InsertPoint(index - 1, pt);
        CreateMovablePoints_();
        UpdateLine_(true);
        profile_changed_.Notify(profile_);
    }
}

void ProfilePane::Impl_::NewPointDragged_(const DragInfo *info, bool is_start) {
    // Note that is_start is true for the start of a drag and info is null for
    // the end of a drag.
    if (is_start) {
        ASSERT(info);
        const Point2f pp = ToProfile_(Point3f(new_point_->GetTranslation()));
        int index = GetNewPointIndex_(pp);
        if (index > 0) {
            new_point_->SetEnabled(false);
            CreateDelegateSlider_(index - 1, pp);
            delegate_slider_ = GetMovableSlider_(index - 1);
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

    if (is_activation) {
        // Detect point motion.
        slider->GetValueChanged().EnableObserver(this, true);

        // Put the delete spot in a good location unless this is a new point or
        // the minimum would be violated.
        if (! delegate_slider_ &&
            profile_.GetPoints().size() > min_point_count_) {
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
            profile_changed_.Notify(profile_);
        }
        delete_spot_->SetEnabled(false);
    }
    activation_.Notify(is_activation);
}

void ProfilePane::Impl_::PointMoved_(size_t index, const Point2f &pos) {
    // Update the point in the Profile.
    ASSERT(index < profile_.GetPoints().size());
    profile_.SetPoint(index, pos);
    UpdateLine_(false);
    profile_changed_.Notify(profile_);

    // Highlight the delete spot if the point is over it.
    if (delete_spot_->IsEnabled())
        delete_spot_->SetActive(delete_rect_.ContainsPoint(pos));
}

void ProfilePane::Impl_::UpdateLine_(bool update_points) {
    // Convert all profile points to 3D positions.
    const std::vector<Point3f> pts =
        Util::ConvertVector<Point3f, Point2f>(
            profile_.GetAllPoints(),
            [](const Point2f &p){ return FromProfile_(p, TK::kPaneZOffset); });

    // Update the line to connect all points.
    profile_line_->SetPoints(pts);

    // Position the movable points based on the interior points if requested.
    if (update_points) {
        for (size_t index = 0; index < profile_.GetPoints().size(); ++index) {
            auto slider = GetMovableSlider_(index);
            const auto &pt = pts[index + 1];  // Skip start point.
            slider->SetValue(Vector2f(pt[0], pt[1]));
        }
    }
}

void ProfilePane::Impl_::CreateDelegateSlider_(size_t index,
                                               const Point2f &pos) {
    profile_.InsertPoint(index, pos);
    CreateMovablePoints_();
    UpdateLine_(true);
}

Slider2DWidgetPtr ProfilePane::Impl_::GetMovableSlider_(size_t index) const {
    ASSERT(index < movable_parent_->GetChildCount());
    Slider2DWidgetPtr slider =
        Util::CastToDerived<Slider2DWidget>(movable_parent_->GetChild(index));
    ASSERT(slider);
    return slider;
}

int ProfilePane::Impl_::GetNewPointIndex_(const Point2f &pt) {
    using ion::math::Distance;

    const auto &points = profile_.GetAllPoints();

    // The point has to be on the profile line but not too close to an existing
    // profile point.
    int on_line_index = -1;
    for (size_t i = 0; i < points.size(); ++i) {
        if (Distance(pt, points[i]) <= TK::kProfilePanePointTolerance)
            return -1;
        if (i > 0 && IsPointOnSegment_(pt, points[i - 1], points[i]))
            on_line_index = i;
    }
    return on_line_index;
}

bool ProfilePane::Impl_::IsPointOnSegment_(const Point2f &p,
                                           const Point2f &end0,
                                           const Point2f &end1) {
    using ion::math::Distance;
    using ion::math::Dot;
    using ion::math::Normalized;

    // Do this in 3D since the function already exists.
    Point3f close0, close1;
    if (! GetClosestLinePoints(Point3f(p, 0), Vector3f(0, 0, 1),
                               Point3f(end0, 0),
                               Normalized(Vector3f(end1 - end0, 0)),
                               close0, close1))
        return false;  // Should never happen - can't be parallel.

    auto to2 = [](const Point3f &p){ return Point2f(p[0], p[1]); };
    const Point2f c0 = to2(close0);
    const Point2f c1 = to2(close1);
    if (Distance(c0, c1) <= TK::kProfilePanePointTolerance) {
        // Make sure the point is between end0 and end1.
        if (Dot(c1 - end0, end1 - end0) > 0 && Dot(c1 - end1, end0 - end1) > 0)
            return true;
    }

    return false;
}

void ProfilePane::Impl_::PositionDeleteSpot_() {
    // Use one of the points in the center of one of the sides, a quarter way
    // in. Choose the one that is farthest from all profile points.
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
        GetClosestPoint_(profile_.GetPoints(), cpts[i], dist);
        if (dist > max_dist) {
            max_index = i;
            max_dist  = dist;
        }
    }
    ASSERT(max_index >= 0 && max_index <= 3);
    const Point2f &pt = cpts[max_index];

    // Position the rectangle and the feedback rectangle.
    PositionDeleteRect_(pt);
    delete_spot_->SetTranslation(FromProfile_(pt, 0));
}

size_t ProfilePane::Impl_::GetClosestPoint_(const std::vector<Point2f> &points,
                                            const Point2f &p, float &dist) {
    ASSERT(! points.empty());
    int closest = 0;
    dist = std::numeric_limits<float>::max();
    for (size_t i = 0; i < points.size(); ++i) {
        const float d = ion::math::Distance(p, points[i]);
        if (d < dist) {
            closest = i;
            dist    = d;
        }
    }
    return closest;
}

void ProfilePane::Impl_::GetClosestMidPoint_(const std::vector<Point2f> &points,
                                             const Point2f &p,
                                             Point2f &mid_pt, float &dist) {
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

// ----------------------------------------------------------------------------
// ProfilePane functions.
// ----------------------------------------------------------------------------

ProfilePane::ProfilePane() {
}

void ProfilePane::AddFields() {
    AddField(min_point_count_.Init("min_point_count", 0));

    LeafPane::AddFields();
}

bool ProfilePane::IsValid(std::string &details) {
    if (! LeafPane::IsValid(details))
        return false;
    if (min_point_count_ < 0) {
        details = "Negative minimum point count";
        return false;
    }
    return true;
}

void ProfilePane::CreationDone() {
    LeafPane::CreationDone();
    if (! IsTemplate())
        impl_.reset(new Impl_(*this, min_point_count_));
}

Util::Notifier<bool> & ProfilePane::GetActivation() {
    return impl_->GetActivation();
}

Util::Notifier<const Profile &> & ProfilePane::GetProfileChanged() {
    return impl_->GetProfileChanged();
}

void ProfilePane::SetProfile(const Profile &profile) {
    ASSERT(profile.GetPoints().size() >= GetMinPointCount());
    impl_->SetProfile(profile);
}

const Profile & ProfilePane::GetProfile() const {
    return impl_->GetProfile();
}

void ProfilePane::SetLayoutSize(const Vector2f &size) {
    LeafPane::SetLayoutSize(size);
    impl_->AdjustSize(GetBaseSize(), size);
}

ClickableWidgetPtr ProfilePane::GetGripWidget(const Point2f &p) {
    return impl_->GetGripWidget(p);
}

IPaneInteractor * ProfilePane::GetInteractor() {
    return this;
}

BorderPtr ProfilePane::GetFocusBorder() const {
    // Cannot take focus - would not make sense.
    return BorderPtr();
}
