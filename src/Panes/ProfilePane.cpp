#include "Panes/ProfilePane.h"

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

    void CreateMovablePoints_();
    void PointActivated_(size_t index, bool is_activation);
    void PointMoved_(size_t index, const Point2f &pos);
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

    CreateMovablePoints_();
    //XXXX
    //UpdateLine_(true);
    //InitDeleteRect_();
}

void ProfilePane::Impl_::SetProfile(const Profile &profile) {
    // XXXX
}

void ProfilePane::Impl_::CreateMovablePoints_() {
    // Create a Slider2DWidget for each movable Profile point. Add it to
    // movable_parent_.
    movable_parent_->ClearChildren();
    for (size_t index = 0; index < profile_.GetPoints().size(); ++index) {
        const std::string name = "MovablePoint_" + Util::ToString(index);
        auto slider = movable_slider_->CloneTyped<Slider2DWidget>(true, name);
        slider->GetActivation().AddObserver(
            this, [&, index](Widget &, bool is_activation){
                PointActivated_(index, is_activation); });
        slider->GetValueChanged().AddObserver(
            this, [&, index](Widget &, const Vector2f &v){
                PointMoved_(index, Point2f(v)); });
        slider->GetValueChanged().EnableObserver(this, false);
        movable_parent_->AddChild(slider);
        slider->SetEnabled(true);
    }
}

void ProfilePane::Impl_::PointActivated_(size_t index, bool is_activation) {
    ASSERT(index < movable_parent_->GetChildCount());

    Slider2DWidgetPtr slider =
        Util::CastToDerived<Slider2DWidget>(movable_parent_->GetChild(index));
    ASSERT(slider);

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
    profile_.SetPoint(index, pos);  // XXXX Need to convert coords???
    // XXXX
    // UpdateLine(_profile, false);
    profile_changed_.Notify(profile_);
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
    impl_->SetProfile(profile);
}

const Profile & ProfilePane::GetProfile() const {
    return impl_->GetProfile();
}
