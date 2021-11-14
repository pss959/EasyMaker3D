#include "GripHandler.h"

#include "Assert.h"
#include "Event.h"
#include "Math/Types.h"
#include "SG/Hit.h"
#include "SG/Intersector.h"
#include "SG/NodePath.h"
#include "SG/Search.h"
#include "Widgets/ClickableWidget.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/Time.h"

namespace {

// ----------------------------------------------------------------------------
// Timer_ class.
// ----------------------------------------------------------------------------

/// Timer_ is used to check for multiple clicks within a chosen duration.
class Timer_ {
  public:
    /// Starts counting the time up to the given duration in seconds. If
    // already running, starts over.
    void Start(double duration) {
        duration_   = duration;
        start_time_ = Util::Time::Now();
    }

    /// Stops counting if it is currently counting. Does nothing if not.
    void Stop() { duration_ = 0; }

    /// Returns true if the timer is running.
    bool IsRunning() const { return duration_ > 0; }

    /// This should be called every frame to check for a finished timer. It
    // returns true if the timer was running and just hit the duration.
    bool IsFinished() {
        if (IsRunning() &&
            Util::Time::Now().SecondsSince(start_time_) >= duration_) {
            duration_ = 0;
            return true;
        }
        return false;
    }

  private:
    double     duration_ = 0;  ///< Set to 0 when not running.
    Util::Time start_time_;
};

// ----------------------------------------------------------------------------
// DeviceData_ struct.
// ----------------------------------------------------------------------------

/// DeviceData_ saves current information about a particular device, including
/// any Widget that it might be interacting with.
struct DeviceData_ {
    Event      event;              ///< Event used to update this data.
    WidgetPtr  activation_widget;  ///< Widget at activation (or null).
    WidgetPtr  cur_widget;         ///< Current Widget being hovered (or null).

    /// \name Pointer Data
    /// These are used only for pointer-based devices.
    ///@{
    Ray        activation_ray;  ///< Pointer ray at activation.
    SG::Hit    activation_hit;  ///< Intersection info at activation.
    Ray        cur_ray;         ///< Current pointer ray.
    SG::Hit    cur_hit;         ///< Current intersection info.
    ///@}

    /// Resets to default state.
    void Reset() {
        event = Event();
        activation_widget.reset();
        cur_widget.reset();
        activation_hit = cur_hit = SG::Hit();
    }
};

// ----------------------------------------------------------------------------
// ClickState_ struct.
// ----------------------------------------------------------------------------

/// ClickState_ saves information about a current potential click in progress.
struct ClickState_ {
    Timer_        timer;      ///< Used to detect multiple clicks.
    int           count = 0;  ///< Current number of clicks.
    Event::Device device;     ///< Device that caused the current click.
    Event::Button button;     ///< Button that was pressed to start the click.

    /// Copy of the Event that causes deactivation (once it is known).
    Event         deactivation_event;

    ClickState_() { Reset(); }

    void Reset() {
        count  = 0;
        device = Event::Device::kUnknown;
        button = Event::Button::kNone;
    }

    /// Returns true if the timer is currently running and the given event uses
    /// the same device and button, meaning this is a multiple click.
    bool IsMultipleClick(Event ev) const {
        return timer.IsRunning() &&
            ev.device == device  && ev.button == button;
    }
};

}  // anonymous namespace

// ----------------------------------------------------------------------------
// GripHandler::Impl_ class.
// ----------------------------------------------------------------------------

class GripHandler::Impl_ {
  public:
    void SetPrecisionManager(const PrecisionManagerPtr &precision_manager) {
        precision_manager_ = precision_manager;
    }
    void SetSceneContext(const SceneContextPtr &context) {
        context_ = context;

        // Save paths to the controllers.
        l_controller_path_ =
            SG::FindNodePathInScene(*context_->scene, context_->left_controller);
        r_controller_path_ =
            SG::FindNodePathInScene(*context_->scene,
                                    context_->right_controller);
        ASSERT(! l_controller_path_.empty());
        ASSERT(! r_controller_path_.empty());
    }
    void AddGrippable(const GrippablePtr &grippable) {
        grippables_.push_back(grippable);
    }
    Util::Notifier<const ClickInfo &> & GetClicked() {
        return clicked_;
    }
    bool IsWaiting() const {
        return state_ == State_::kWaiting && ! click_state_.timer.IsRunning();
    }
    void ProcessUpdate(bool is_alternate_mode);
    bool HandleEvent(const Event &event);
    void Reset();

  private:
    // ------------------------------------------------------------------------
    // Types.

    /// Possible states.
    enum class State_ {
        kWaiting,    ///< Waiting for activation events.
        kActivated,  ///< Activation button pressed, but not dragging.
        kDragging,   ///< Activated and sufficient motion for dragging.
    };

    // ------------------------------------------------------------------------
    // Constants.

    /// Time in seconds to wait for multiple clicks.
    static constexpr float  kClickTimeout_ = .25f;

    /// Minimum time in seconds for a press to be considered a long press.
    static constexpr float  kLongPressTime_ = .6f;

    /// Minimum world-space distance for a controller to move to be considered
    // a potential grip drag operation.
    static constexpr float  kMinDragDistance_ = .04f;

    /// Minimum angle between two ray directions to be considered enough for a
    // drag.
    static const Anglef     kMinRayAngle_;

    // ------------------------------------------------------------------------
    // Variables.

    /// Current state.
    State_ state_ = State_::kWaiting;

    /// PrecisionManager used for accessing precision details.
    PrecisionManagerPtr precision_manager_;

    /// SceneContext the handler is interacting with.
    SceneContextPtr context_;

    /// Path from the scene root to the left controller, used to convert to
    /// local controller coordinates.
    SG::NodePath l_controller_path_;
    /// Path from the scene root to the right controller, used to convert to
    /// local controller coordinates.
    SG::NodePath r_controller_path_;

    /// Ordered set of Grippable instances for interaction.
    std::vector<GrippablePtr> grippables_;

    /// Current Grippable: the first one that is enabled.
    GrippablePtr cur_grippable_;

    /// Path from the Scene root to cur_grippable_. This is used to convert
    /// local coordinates to world coordinates.
    SG::NodePath cur_grippable_path_;

    /// Notifies when a click is detected.
    Util::Notifier<const ClickInfo &> clicked_;

    /// Time at which the current device was activated.
    Util::Time       activation_time_;

    /// Information used to detect and process clicks.
    ClickState_      click_state_;

    /// \name Device Data
    /// Each of these holds the state of a tracked device.
    ///@{
    DeviceData_ l_grip_data_;  ///< Grip data for left controller.
    DeviceData_ r_grip_data_;  ///< Grip data for right controller.
    ///@}

    /// Points to the DeviceData_ instance for the active device, or null if no
    /// device is active.
    DeviceData_      *active_data_;

    /// This is set to true after activation if the device moved enough to be
    // considered a drag operation.
    bool             moved_enough_for_drag_ = false;

    /// DragInfo instance used to process drags.
    DraggableWidget::DragInfo drag_info_;

    // ------------------------------------------------------------------------
    // Functions.

    /// Activates the device in the given event.
    void Activate_(const Event &event);

    /// Updates each DeviceData_ that is affected by the given event. If this
    /// causes a change in hover status, this takes care of that as well.
    void UpdateAllDeviceData_(const Event &event);

    /// Updates the given DeviceData_ instance based on the given event.
    void UpdateDeviceData_(const Event &event, DeviceData_ &data);

    /// Updates the hovering state of the two widgets if necessary.
    static void UpdateHovering_(const WidgetPtr &old_widget,
                                const WidgetPtr &new_widget);

    /// Deactivates the current active device and finishes processing the
    /// current operation, if any.
    void Deactivate_();

    /// Returns true if a drag should start.
    bool ShouldStartDrag_();

    /// Returns true if the active device indicates there was enough motion to
    /// start a drag.
    bool MovedEnoughForDrag_();

    /// Starts or continues a drag operation using the current draggable.
    void ProcessDrag_();

    /// Processes a click using the given device.
    void ProcessClick_(Event::Device device, bool is_alternate_mode);

    /// Resets everything after it is known that a click has finished: the
    /// timer is no longer running.
    void ResetClick_(const Event &event);

    /// Returns a pointer to the DeviceData_ for the device in the given event.
    /// Returns null if the device is not one of the ones the GripHandler cares
    /// about.
    DeviceData_ * GetDeviceData_(const Event &event);

    /// Returns the active widget as a DraggableWidget.
    DraggableWidget * GetDraggable_(bool error_if_not_there = true) {
        DraggableWidget *dw = dynamic_cast<DraggableWidget *>(
            active_data_->activation_widget.get());
        if (error_if_not_there) {
            ASSERT(dw);
        }
        return dw;
    }

    /// Returns true if the two given positions are different enough to begin a
    // drag operation.
    static bool PointMovedEnough_(const Point3f &p0, const Point3f &p1,
                                  bool is_clickable) {
        // Use half the threshhold if the widget is not also clickable.
        const float scale = is_clickable ? 1.f : .5f;
        return ion::math::Distance(p0, p1) > scale * kMinDragDistance_;
    }

    /// Returns true if the two given directions are different enough to
    // begin a drag operation.
    static bool DirectionMovedEnough_(const Vector3f &d0, const Vector3f &d1,
                                      const Anglef &min, bool is_clickable) {
        using ion::math::AngleBetween;
        using ion::math::Normalized;

        // Use half the threshhold if the widget is not also clickable.
        const float scale = is_clickable ? 1.f : .5f;
        return AngleBetween(Normalized(d0), Normalized(d1)) > scale * min;
    }

    /// Returns true if the given event represents activation of a grip.
    static bool IsActivationEvent_(const Event &event) {
        return event.flags.Has(Event::Flag::kButtonPress) &&
            event.button == Event::Button::kGrip;
    }

    /// Returns true if the given event represents deactivation of a device
    // with the given button.
    static bool IsDeactivationEvent_(const Event &event, Event::Button button) {
        return event.flags.Has(Event::Flag::kButtonRelease) &&
            event.button == button;
    }
};

const Anglef GripHandler::Impl_::kMinRayAngle_ = Anglef::FromDegrees(2);

// ----------------------------------------------------------------------------
// GripHandler::Impl_ implementation.
// ----------------------------------------------------------------------------

void GripHandler::Impl_::ProcessUpdate(bool is_alternate_mode) {
    // Determine what the current (enabled) Grippable is, if any.
    const auto prev_grippable = cur_grippable_;
    cur_grippable_.reset();
    for (auto &grippable: grippables_) {
        if (grippable->IsGrippableEnabled()) {
            cur_grippable_ = grippable;
            break;
        }
    }
    // Update the guides in the controllers if the Grippable changed.
    if (cur_grippable_ != prev_grippable) {
        const GripGuideType ggt = cur_grippable_ ?
            cur_grippable_->GetGripGuideType() : GripGuideType::kNone;
        context_->left_controller->SetGripGuideType(ggt);
        context_->right_controller->SetGripGuideType(ggt);

        // Save the path to the Grippable for coordinate conversion.
        if (cur_grippable_)
            cur_grippable_path_ =
                SG::FindNodePathInScene(*context_->scene, cur_grippable_);
    }

    // If the click timer finishes and not in the middle of another click or
    // drag, process the click. If not, then clear _activeData.
    if (click_state_.timer.IsFinished()) {
        if (IsWaiting()) {
            if (click_state_.count > 0)
                ProcessClick_(click_state_.device, is_alternate_mode);
            ResetClick_(click_state_.deactivation_event);
        }
    }
}

bool GripHandler::Impl_::HandleEvent(const Event &event) {
    if (! context_)
        return false;

    if (event.flags.Has(Event::Flag::kPosition3D) &&
        event.flags.Has(Event::Flag::kOrientation)) {
        ControllerPtr controller;
        SG::NodePath  controller_path;
        if (event.device == Event::Device::kLeftController) {
            controller      = context_->left_controller;
            controller_path = l_controller_path_;
        }
        else {
            controller      = context_->right_controller;
            controller_path = r_controller_path_;
        }

        if (cur_grippable_) {
            GripInfo info;
            info.event = event;
            cur_grippable_->UpdateGripInfo(info);
            if (info.widget) {
                const Point3f p = controller_path.ToLocal(
                    cur_grippable_path_.FromLocal(info.target_point));
                controller->ShowGripHover(p, info.color);
            }
            else
                controller->HideGripHover();
        }
        else {
            controller->HideGripHover();
        }
    }

    return false;  // No need to steal event.
}

void GripHandler::Impl_::Reset() {
    l_grip_data_.Reset();
    r_grip_data_.Reset();

    click_state_.Reset();

    state_ = State_::kWaiting;
    active_data_ = nullptr;
    moved_enough_for_drag_ = false;
}

#if XXXX
void GripHandler::Impl_::UpdateGrippable_() {
    _curGrippable = _grippables.Find((g) => g.IsGrippableEnabled());

    // Update the grip guide.
    _deviceManager.SetGripGuideType(
        _curGrippable == null ? GripGuide.GuideType.None :
        _curGrippable.GetGripGuideType());
}
#endif

void GripHandler::Impl_::Activate_(const Event &event) {
    // XXXX
    std::cerr << "XXXX GripHandler Activated\n";
}

void GripHandler::Impl_::UpdateAllDeviceData_(const Event &event) {
#if XXXX
    // Don't change the hovering state if waiting for the end of a click, since
    // that could mess up the active state.
    const bool update_hover = ! click_state_.timer.IsRunning();

    if (event.flags.Has(Event::Flag::kPosition3D)) {
        if (DeviceData_ *data = GetDeviceData_(event)) {
            WidgetPtr old_widget = data->cur_widget;
            UpdateDeviceData_(event, *data);
            if (update_hover) {
                UpdateHovering_(old_widget, data->cur_widget);
                controller->ShowPointerHover(data->point);
            }
            GameObject oldGO = data.go;

        // Assume no target object and assume default color.
        data.go = null;
        ev.gripData.color = Color.white;

        // Ask the IGrippable, if any, for the new interactive GameObject.
        if (_curGrippable != null) {
            _curGrippable.UpdateGripDeviceData(ev.gripData);
            data.go    = ev.gripData.go;
            data.point = ev.gripData.targetPoint;
        }

        // Update the _DeviceData.
        data.device = dev;
        data.isGrip = true;
        data.hadGO  = data.go != null;

        UpdateHovering(oldGO, data.go);

        // Let the device know.
        device_manager->ShowGripHover(data.hadGO, data.point,
                                      event.gripData.color);
    }
#endif
}

#if XXXX
void GripHandler::Impl_::UpdateDeviceData_(const Event &event,
                                           DeviceData_ &data) {
    data.event = event;

    // Ask the current Grippable for the grip target.
    if (cur_grippable_) {
        GripInfo info;
        info.event = event;
        cur_grippable_->UpdateGripInfo(info);
        data.cur_widget = info.widget;
    }
}
#endif

void GripHandler::Impl_::UpdateHovering_(const WidgetPtr &old_widget,
                                         const WidgetPtr &new_widget) {
    if (old_widget != new_widget ||
        (! new_widget ||  ! new_widget->IsHovering())) {
        if (old_widget)
            old_widget->SetHovering(false);
        if (new_widget)
            new_widget->SetHovering(true);
    }
}

void GripHandler::Impl_::Deactivate_() {
    ASSERT(active_data_);
    const Event &event = active_data_->event;

    if (active_data_->activation_widget)
        active_data_->activation_widget->SetActive(false);

    if (state_ == State_::kDragging)
        GetDraggable_()->EndDrag();

    if (click_state_.timer.IsRunning()) {
        // If the timer is still running, save the deactivation event.
        click_state_.deactivation_event = event;
    }
    else {
        // The deactivation represents a click if all of the following are
        // true:
        //   - A drag is not in process.
        //   - The active device did not move too much (enough for a drag).
        //   - The active device did not move off the clickable widget.
        const bool is_click = state_ != State_::kDragging &&
            ! moved_enough_for_drag_ &&
            active_data_->cur_widget == active_data_->activation_widget;

        // If the timer is not running, process the click if it is one, and
        // always reset everything.
        if (is_click)
            ProcessClick_(event.device, event.is_alternate_mode);
        ResetClick_(event);
    }
    state_ = State_::kWaiting;
    KLOG('h', "GripHandler kWaiting after deactivation");
}

bool GripHandler::Impl_::ShouldStartDrag_() {
    ASSERT(active_data_);
    ASSERT(state_ == State_::kActivated);
    moved_enough_for_drag_ = MovedEnoughForDrag_();

    return GetDraggable_(false) && moved_enough_for_drag_;
}

bool GripHandler::Impl_::MovedEnoughForDrag_() {
    ASSERT(active_data_);

    const auto &ev = active_data_->event;

    const bool is_clickable = Util::CastToDerived<ClickableWidget>(
        active_data_->activation_widget).get();
    // Check for position change.
    if (PointMovedEnough_(active_data_->activation_ray.origin,
                          ev.position3D, is_clickable))
        return true;

    // Always do a ray-based test.
    const Vector3f ray_dir = ev.orientation * Vector3f::AxisZ();
    // Need more rotation for grip drags.
    const Anglef threshold = 5 * kMinRayAngle_;
    return DirectionMovedEnough_(active_data_->activation_ray.direction,
                                 ray_dir, threshold, is_clickable);
}

void GripHandler::Impl_::ProcessDrag_() {
    ASSERT(state_ == State_::kActivated || state_ == State_::kDragging);
    ASSERT(active_data_);
    ASSERT(moved_enough_for_drag_);

    drag_info_.is_grip_drag = true;
    drag_info_.is_alternate_mode =
        active_data_->event.is_alternate_mode || click_state_.count > 1;
    drag_info_.linear_precision  = precision_manager_->GetLinearPrecision();
    drag_info_.angular_precision = precision_manager_->GetAngularPrecision();

    if (state_ == State_::kActivated) {
        // Start of a new drag.
        drag_info_.hit = active_data_->activation_hit;
        if (active_data_->activation_widget)
            active_data_->activation_widget->SetHovering(false);
        auto draggable = GetDraggable_();
        draggable->StartDrag(drag_info_);
        state_ = State_::kDragging;
        KLOG('h', "GripHandler kDragging with " << draggable->GetDesc());
    }
    else {
        // Continuation of current drag.
        ASSERT(state_ == State_::kDragging);
        drag_info_.hit = active_data_->cur_hit;
        GetDraggable_()->ContinueDrag(drag_info_);
    }
}

void GripHandler::Impl_::ProcessClick_(Event::Device device,
                                       bool is_alternate_mode) {
    ASSERT(active_data_);

    ClickInfo info;
    info.hit               = active_data_->cur_hit;
    info.is_alternate_mode = is_alternate_mode || click_state_.count > 1;
    info.is_long_press     =
        Util::Time::Now().SecondsSince(activation_time_) > kLongPressTime_;

    info.widget = Util::CastToDerived<ClickableWidget>(
        active_data_->activation_widget).get();

    clicked_.Notify(info);

    state_ = State_::kWaiting;
    KLOG('h', "GripHandler kWaiting after click on "
         << info.hit.path.ToString());
}

void GripHandler::Impl_::ResetClick_(const Event &event) {
    ASSERT(! click_state_.timer.IsRunning());
    //if (click_state_.device != Event::Device::kUnknown)
    // XXXX_deviceManager.SetDeviceActive(_clickState.device, false, event);
    active_data_ = nullptr;
    click_state_.Reset();
}

#if XXXX
DeviceData_ * GripHandler::Impl_::GetDeviceData_(const Event &event) {
    if (event.device == Event::Device::kLeftController)
        return &l_grip_data_;
    else if (event.device == Event::Device::kRightController)
        return &r_grip_data_;
    else
        return nullptr;
}
#endif

// ----------------------------------------------------------------------------
// GripHandler functions.
// ----------------------------------------------------------------------------

GripHandler::GripHandler() : impl_(new Impl_) {
}

GripHandler::~GripHandler() {
}

void GripHandler::SetPrecisionManager(
    const PrecisionManagerPtr &precision_manager) {
    impl_->SetPrecisionManager(precision_manager);
}

void GripHandler::SetSceneContext(const SceneContextPtr &context) {
    impl_->SetSceneContext(context);
}

void GripHandler::AddGrippable(const GrippablePtr &grippable) {
    impl_->AddGrippable(grippable);
}

Util::Notifier<const ClickInfo &> & GripHandler::GetClicked() {
    return impl_->GetClicked();
}

bool GripHandler::IsWaiting() const {
    return impl_->IsWaiting();
}

void GripHandler::ProcessUpdate(bool is_alternate_mode) {
    impl_->ProcessUpdate(is_alternate_mode);
}

bool GripHandler::HandleEvent(const Event &event) {
    return impl_->HandleEvent(event);
}

void GripHandler::Reset() {
    impl_->Reset();
}
