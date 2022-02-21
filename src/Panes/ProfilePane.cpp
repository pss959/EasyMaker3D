#include "Panes/ProfilePane.h"

#include <ion/math/vectorutils.h>

#include "ClickInfo.h"
#include "Math/Linear.h"
#include "SG/PolyLine.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/String.h"
#include "Widgets/GenericWidget.h"
#include "Widgets/NewPointWidget.h"
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

  private:
    /// Maximum distance to a point to be considered close to it.
    static constexpr float kCloseToPointDistance = .04f;

    const size_t min_point_count_;

    /// Profile being edited.
    Profile profile_;

    /// Notifies when any interaction starts or ends.
    Util::Notifier<bool> activation_;

    /// Notifies when any interactive change is made to the Profile.
    Util::Notifier<const Profile &> profile_changed_;

    // Parts
    SG::NodePtr       start_point_;     ///< Fixed start point indicator.
    SG::NodePtr       end_point_;       ///< Fixed end point indicator.
    SG::NodePtr       delete_spot_;     ///< Target spot for deleting points.
    SG::NodePtr       movable_parent_;  ///< Parent of movable point widgets.
    Slider2DWidgetPtr movable_slider_;  ///< Slider cloned per movable point.
    GenericWidgetPtr  area_widget_;     ///< Detects drags in Pane area.
    SG::PolyLinePtr   profile_line_;    ///< Line showing Profile.

    /// Widget used to handle drags when creating a new point.
    NewPointWidgetPtr  new_point_widget_;

    /// Rectangle representing the drag target area used for deleting points.
    Range2f         delete_rect_;

    /// When the Profile line is dragged to create a new point, this stores the
    /// Slider2DWidget that the rest of the drag is delegated to. It is null
    /// the rest of the time.
    Slider2DWidgetPtr delegate_slider_;

    void PositionFixedPoints_();
    void CreateMovablePoints_();
    void InitDeleteRect_();
    void AreaClicked_(const ClickInfo &info);
    void AreaDragged_(const DragInfo *info, bool is_start);
    void PointActivated_(size_t index, bool is_activation);
    void PointMoved_(size_t index, const Point2f &pos);
    void UpdateLine_(bool update_points);
    Slider2DWidgetPtr GetMovableSlider_(size_t index);

    /// If the given point is on the profile line but not too close to any
    // existing point, this returns the index (> 0) into the full set of
    // profile points (including the fixed points at the ends) of where to
    // create a new point. Otherwise, it returns -1.
    int GetNewPointIndex_(const Point2f &pt);

    void AddProfilePoint_(size_t index, const Point2f &pos);
    static bool IsPointOnSegment_(const Point2f &p,
                                  const Point2f &end0, const Point2f &end1);

    /// Converts a 3D point from the object coordinates of the ProfilePane to
    /// 2D profile coordinates.
    static Point2f ToProfile_(const Point3f &p) {
        // Convert X and Y from (-.5,.5) to (0,1).
        return Point2f(.5f + p[0], .5f + p[1]);
    }
    static Point2f ToProfile_(const Vector2f &p) {
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
    min_point_count_(min_point_count) {

    // Find all the parts.
    start_point_    = SG::FindNodeUnderNode(root_node, "StartPoint");
    end_point_      = SG::FindNodeUnderNode(root_node, "EndPoint");
    delete_spot_    = SG::FindNodeUnderNode(root_node, "DeleteSpot");
    movable_parent_ = SG::FindNodeUnderNode(root_node, "MovableParent");
    movable_slider_ = SG::FindTypedNodeUnderNode<Slider2DWidget>(
        root_node, "MovableSlider");
    area_widget_    = SG::FindTypedNodeUnderNode<GenericWidget>(
        root_node, "AreaWidget");

    auto line = SG::FindNodeUnderNode(root_node, "ProfileLine");
    ASSERT(line->GetShapes().size() == 1U);
    profile_line_ =  Util::CastToDerived<SG::PolyLine>(line->GetShapes()[0]);
    ASSERT(profile_line_);

    area_widget_->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){ AreaClicked_(info); });
    area_widget_->GetDragged().AddObserver(
        this, [&](const DragInfo *info,
                  bool is_start){ AreaDragged_(info, is_start); });

    PositionFixedPoints_();
    CreateMovablePoints_();
    UpdateLine_(true);
    InitDeleteRect_();
}

void ProfilePane::Impl_::SetProfile(const Profile &profile) {
    profile_ = profile;

    PositionFixedPoints_();
    CreateMovablePoints_();
    UpdateLine_(true);
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

            // Use the same range as Pane coordinates: (-.5,.5).
            slider->SetRange(Vector2f(-.5f, -.5f), Vector2f(.5f, .5f));
            slider->GetActivation().AddObserver(
                this, [&, index](Widget &, bool is_activation){
                    PointActivated_(index, is_activation); });
            slider->GetValueChanged().AddObserver(
                this, [&, index](Widget &, const Vector2f &v){
                    PointMoved_(index, ToProfile_(v)); });
            slider->GetValueChanged().EnableObserver(this, false);
            movable_parent_->AddChild(slider);
            slider->SetEnabled(true);
        }
    }
}

void ProfilePane::Impl_::InitDeleteRect_() {
    const Vector3f size = delete_spot_->GetBounds().GetSize();
    delete_rect_ = BuildRange(Point2f::Zero(), Vector2f(size[0], size[1]));
}

void ProfilePane::Impl_::AreaClicked_(const ClickInfo &info) {
    const Point2f pt = ToProfile_(info.hit.point);
    const int index = GetNewPointIndex_(pt);
    if (index > 0) {
        AddProfilePoint_(index - 1, pt);
        CreateMovablePoints_();
        UpdateLine_(true);
        profile_changed_.Notify(profile_);
    }
}

void ProfilePane::Impl_::AreaDragged_(const DragInfo *info, bool is_start) {
    std::cerr << "XXXX AreaDragged_!\n";
}

void ProfilePane::Impl_::PointActivated_(size_t index, bool is_activation) {
    auto slider = GetMovableSlider_(index);

    if (is_activation) {
        // Detect point motion.
        slider->GetValueChanged().EnableObserver(this, true);

#if XXXX
        // Put the delete spot in a good location unless this is a new
        // point or the minimum would be violated.
        if (_delegateWidget == null &&
            _profile.GetPointCount() > _minPointCount) {
            PositionDeleteSpot(_profile);
            _deleteSpotGO.SetActive(true);
        }
#endif
    }
    else {
        // Stop tracking point motion.
        slider->GetValueChanged().EnableObserver(this, false);

#if XXXX
        // If the point was dragged over the delete spot, delete it.
        if (_deleteSpotGO.activeSelf &&
            _deleteRect.Contains(_profile.points[index])) {
            _profile.points.RemoveAt(index);
        }
        _deleteSpotGO.SetActive(false);
        Changed.Invoke(_profile);
#endif
    }
    activation_.Notify(is_activation);
}

void ProfilePane::Impl_::PointMoved_(size_t index, const Point2f &pos) {
    // Update the point in the Profile.
    ASSERT(index < profile_.GetPoints().size());
    profile_.SetPoint(index, pos);
    UpdateLine_(false);
    profile_changed_.Notify(profile_);
}

void ProfilePane::Impl_::UpdateLine_(bool update_points) {
    // Convert all profile points to 3D positions.
    const std::vector<Point3f> pts =
        Util::ConvertVector<Point3f, Point2f>(
            profile_.GetAllPoints(),
            [](const Point2f &p){ return FromProfile_(p, .01f); });

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

Slider2DWidgetPtr ProfilePane::Impl_::GetMovableSlider_(size_t index) {
    ASSERT(index < movable_parent_->GetChildCount());
    Slider2DWidgetPtr slider =
        Util::CastToDerived<Slider2DWidget>(movable_parent_->GetChild(index));
    ASSERT(slider);
    return slider;
}

int ProfilePane::Impl_::GetNewPointIndex_(const Point2f &pt) {
    const auto &points = profile_.GetAllPoints();

    // The point has to be on the profile line but not too close to an existing
    // profile point.
    int on_line_index = -1;
    for (size_t i = 0; i < points.size(); ++i) {
        if (ion::math::Distance(pt, points[i]) <= kCloseToPointDistance)
            return -1;
        if (i > 0 && IsPointOnSegment_(pt, points[i - 1], points[i]))
            on_line_index = i;
    }
    return on_line_index;
}

void ProfilePane::Impl_::AddProfilePoint_(size_t index, const Point2f &pos) {
    profile_.InsertPoint(index, pos);
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
    if (Distance(c0, c1) <= kCloseToPointDistance) {
        // Make sure the point is between end0 and end1.
        if (Dot(c1 - end0, end1 - end0) > 0 && Dot(c1 - end1, end0 - end1) > 0)
            return true;
    }

    return false;
}

// ----------------------------------------------------------------------------
// ProfilePane functions.
// ----------------------------------------------------------------------------

ProfilePane::ProfilePane() {
}

void ProfilePane::AddFields() {
    AddField(min_point_count_);
    Pane::AddFields();
}

bool ProfilePane::IsValid(std::string &details) {
    if (! Pane::IsValid(details))
        return false;
    if (min_point_count_ < 0) {
        details = "Negative minimum point count";
        return false;
    }
    return true;
}

void ProfilePane::CreationDone() {
    Pane::CreationDone();
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
