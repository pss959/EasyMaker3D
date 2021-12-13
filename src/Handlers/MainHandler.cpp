#include "MainHandler.h"

#include "Debug/ShowHit.h"
#include "Event.h"
#include "Items/Controller.h"
#include "Math/Types.h"
#include "SG/Hit.h"
#include "SG/Intersector.h"
#include "SG/Search.h"
#include "Widgets/ClickableWidget.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/Time.h"

namespace {

// ----------------------------------------------------------------------------
// Device_ enum.
// ----------------------------------------------------------------------------

/// Devices that can cause clicking or dragging.
enum class Device_ {
    kNone,
    kMouse,
    kLeftPinch,
    kRightPinch,
    kLeftGrip,
    kRightGrip,
};

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
    /// Device_ this is for.
    Device_       device       = Device_::kNone;
    /// Corresponding Event::Device.
    Event::Device event_device = Event::Device::kUnknown;
    /// Corresponding Controller, if any.
    ControllerPtr controller;
    ///< Widget last hovered by this Device_ (may be null).
    WidgetPtr     hovered_widget;
    /// Widget in active use by this Device_ (may be null).
    WidgetPtr     active_widget;

    /// \name Ray data
    /// For pointer devices (mouse and pinch), the ray corresponds to the
    /// pointing origin and direction. For a grip device, the ray origin is the
    /// controller position and the ray direction is the current grip guide
    /// direction (which includes controller orientation).
    ///@{
    Ray activation_ray;  ///< Ray at activation.
    Ray cur_ray;         ///< Current ray.
    ///@}

    /// \name Pointer data
    /// These store ray intersection data For pointer devices (mouse and
    /// pinch). They are unused for grip devices.
    ///@{
    SG::Hit activation_hit;  ///< Intersection info for activation_ray.
    SG::Hit cur_hit;         ///< Intersection info for cur_ray.
    ///@}

    /// \name Grip data
    /// These store Grippable::GripInfo data for grip devices.
    ///@{
    Grippable::GripInfo activation_grip_info;  ///< Grip info at activation.
    Grippable::GripInfo cur_grip_info;         ///< Grip info for current state.
    ///@}

    /// Resets to default state.
    void Reset() {
        active_widget.reset();
        hovered_widget.reset();
        activation_hit       = cur_hit       = SG::Hit();
        activation_grip_info = cur_grip_info = Grippable::GripInfo();
    }

    /// Convenience that returns true if the device represents a grip.
    bool IsGrip() const {
        return device == Device_::kLeftGrip || device == Device_::kRightGrip;
    }
};

// ----------------------------------------------------------------------------
// ClickState_ struct.
// ----------------------------------------------------------------------------

/// ClickState_ saves information about a current potential click in progress.
struct ClickState_ {
    Timer_        timer;      ///< Used to detect multiple clicks.
    int           count = 0;  ///< Current number of clicks.
    Device_       device;     ///< Device that caused the current click.
    Event::Button button;     ///< Button that was pressed to start the click.

    /// Copy of the Event that causes deactivation (once it is known).
    Event         deactivation_event;

    ClickState_() { Reset(); }

    void Reset() {
        count  = 0;
        device = Device_::kNone;
        button = Event::Button::kNone;
    }

    /// Returns true if the timer is currently running and the passed device
    /// and button match what is stored here, meaning this is a multiple click.
    bool IsMultipleClick(Device_ dev, Event::Button event_button) const {
        return timer.IsRunning() && dev == device  && event_button == button;
    }
};

}  // anonymous namespace

// ----------------------------------------------------------------------------
// MainHandler::Impl_ class.
// ----------------------------------------------------------------------------

class MainHandler::Impl_ {
  public:
    Impl_();
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

        auto set_controller = [this](Device_ dev, ControllerPtr controller){
            GetDeviceData_(dev).controller = controller;
        };
        set_controller(Device_::kLeftPinch,  context_->left_controller);
        set_controller(Device_::kLeftGrip,   context_->left_controller);
        set_controller(Device_::kRightPinch, context_->right_controller);
        set_controller(Device_::kRightGrip,  context_->right_controller);
    }
    void AddGrippable(const GrippablePtr &grippable) {
        grippables_.push_back(grippable);
    }
    Util::Notifier<const ClickInfo &> & GetClicked() {
        return clicked_;
    }
    Util::Notifier<Event::Device, float> & GetValuatorChanged() {
        return valuator_changed_;
    }
    bool IsWaiting() const {
        return state_ == State_::kWaiting && ! click_state_.timer.IsRunning();
    }
    void SetPathFilter(const PathFilter &filter) { path_filter_ = filter; }
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

    /// Notifies when a valuator change is detected.
    Util::Notifier<Event::Device, float> valuator_changed_;

    /// Time at which the current device was activated.
    Util::Time       activation_time_;

    /// Information used to detect and process clicks.
    ClickState_      click_state_;

    /// Function passed to SetPathFilter().
    PathFilter       path_filter_;

    /// DeviceData_ for each Device_ (including Device_::kNone for simplicity).
    DeviceData_      device_data_[Util::EnumCount<Device_>()];

    /// Device_ that is currently active, possibly Device_::kNone.
    Device_          active_device_ = Device_::kNone;

    /// This is set to true after activation if the device moved enough to be
    // considered a drag operation.
    bool             moved_enough_for_drag_ = false;

    /// DragInfo instance used to process drags.
    DraggableWidget::DragInfo drag_info_;

    // ------------------------------------------------------------------------
    // Functions.

    /// Activates a device based on the given event.
    void Activate_(Device_ dev, const Event &event);

    /// Deactivates the current active device and finishes processing the
    /// current operation, if any.
    void Deactivate_(const Event &event);

    /// This is called when the handler is activated or dragging. It checks the
    /// given event for both the start of a new drag or continuation of a
    /// current drag. It returns true if either was true.
    bool StartOrContinueDrag_(const Event &event);

    /// Updates all relevant DeviceData_ instances based on the given event. If
    /// dev is not Device_::kNone, this restricts the update to that device. If
    /// update_hover is true, this also updates the hovering status for all
    /// relevant Widgets.
    void UpdateDeviceData_(const Event &event, Device_ dev, bool update_hover);

    /// Updates the current Ray and Hit in the DeviceData_ for the given
    /// Device_ based on the given Ray and its intersection.
    void UpdatePointerData_(const Event &event, Device_ dev, const Ray &ray,
                            bool update_hover);

    /// Updates the current Ray and Grippable::GripInfo in the DeviceData_ for
    /// the given Device_ based on the current controller state.
    void UpdateGripData_(const Event &event, Device_ dev, bool update_hover);

    /// Updates the hovering state of the two widgets if necessary.
    void UpdateWidgetHover_(const WidgetPtr &old_widget,
                            const WidgetPtr &new_widget);

    /// Returns true if a drag should start.
    bool ShouldStartDrag_();

    /// Returns true if the active device indicates there was enough motion to
    /// start a drag.
    bool MovedEnoughForDrag_();

    /// Starts or continues a drag operation using the current draggable.
    void ProcessDrag_(bool is_alternate_mode);

    /// Processes a click using the given device.
    void ProcessClick_(Device_ dev, bool is_alternate_mode);

    /// Resets everything after it is known that a click has finished: the
    /// timer is no longer running.
    void ResetClick_(const Event &event);

    /// Returns the DeviceData_ for the given device.
    DeviceData_ & GetDeviceData_(Device_ dev) {
        return device_data_[Util::EnumInt(dev)];
    }

    /// Converts a point from world coordinates into local coordinates for the
    /// controller with the given Hand.
    Point3f ToLocalControllerCoords(Hand hand, const Point3f &p) {
        const auto &path = hand == Hand::kLeft ?
            l_controller_path_ : r_controller_path_;
        return path.ToLocal(p);
    }

    /// Returns true if the two given positions are different enough to begin a
    // drag operation.
    static bool PointMovedEnough_(const Point3f &p0, const Point3f &p1,
                                  bool is_clickable);
    /// Returns true if the two given directions are different enough to
    // begin a drag operation.
    static bool DirectionMovedEnough_(const Vector3f &d0, const Vector3f &d1,
                                      const Anglef &min, bool is_clickable);

    /// Returns the Device_ corresponding to a button press or release Event,
    /// possibly Device_::kNone.
    static Device_ GetDeviceForButtonEvent_(const Event &event);
};

const Anglef MainHandler::Impl_::kMinRayAngle_ = Anglef::FromDegrees(2);

// ----------------------------------------------------------------------------
// MainHandler::Impl_ implementation.
// ----------------------------------------------------------------------------

MainHandler::Impl_::Impl_() {
    for (auto dev: Util::EnumValues<Device_>()) {
        DeviceData_ &ddata = GetDeviceData_(dev);
        ddata.device = dev;
        switch (dev) {
          case Device_::kMouse:
            ddata.event_device = Event::Device::kMouse;
            break;
          case Device_::kLeftPinch:
          case Device_::kLeftGrip:
            ddata.event_device = Event::Device::kLeftController;
            break;
          case Device_::kRightPinch:
          case Device_::kRightGrip:
            ddata.event_device = Event::Device::kRightController;
            break;
          default:
            ddata.event_device = Event::Device::kUnknown;
            break;
        }
    }
}

void MainHandler::Impl_::ProcessUpdate(bool is_alternate_mode) {
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

bool MainHandler::Impl_::HandleEvent(const Event &event) {
    if (! context_)
        return false;

    bool handled = false;

    // Handle valuator events first.
    if (event.flags.Has(Event::Flag::kPosition1D)) {
        valuator_changed_.Notify(event.device, event.position1D);
        handled = true;
    }

    switch (state_) {
      case State_::kWaiting:
        // Waiting for something to happen. If there is an activation event
        // (button press on an enabled Widget), activate the corresponding
        // device.
        if (active_device_ == Device_::kNone) {
            const Device_ dev = event.flags.Has(Event::Flag::kButtonPress) ?
                GetDeviceForButtonEvent_(event) : Device_::kNone;
            if (dev != Device_::kNone) {
                Activate_(dev, event);
                handled = true;
            }
        }
        // Update the state for all devices unless waiting for the end of a
        // click; that could mess up the active state.
        if (! handled && ! click_state_.timer.IsRunning())
            UpdateDeviceData_(event, Device_::kNone, true);

        break;

      case State_::kActivated:
      case State_::kDragging:
        ASSERT(active_device_ != Device_::kNone);
        // Check for a deactivation of the active device.
        if (event.flags.Has(Event::Flag::kButtonRelease) &&
            GetDeviceForButtonEvent_(event) == active_device_) {
            Deactivate_(event);
            handled = true;
        }
        else {
            // Not a deactivation? See if this starts or continues a drag.
            handled = StartOrContinueDrag_(event);
        }
        break;
    }
#if DEBUG && 0
    context_->debug_text->SetText(Util::EnumName(state_) + " / " +
                                  Util::EnumName(active_device_));
#endif

    return handled;
}

void MainHandler::Impl_::Reset() {
    for (size_t i = 0; i < Util::EnumCount<Device_>(); ++i)
        device_data_[i].Reset();

    click_state_.Reset();

    state_                 = State_::kWaiting;
    active_device_         = Device_::kNone;
    moved_enough_for_drag_ = false;
}

void MainHandler::Impl_::Activate_(Device_ dev, const Event &event) {
    ASSERT(active_device_ == Device_::kNone);

    // Update the active DeviceData_.
    active_device_ = dev;
    DeviceData_ &ddata = GetDeviceData_(active_device_);

    // Update the data for the active device from the event.
    UpdateDeviceData_(event, active_device_, false);
    ddata.active_widget  = ddata.hovered_widget;
    ddata.activation_ray = ddata.cur_ray;
    if (ddata.IsGrip()) {
        ddata.activation_grip_info = ddata.cur_grip_info;
        if (cur_grippable_)
            cur_grippable_->ActivateGrip(ddata.controller->GetHand(), true);
    }
    else {
        ddata.activation_hit = ddata.cur_hit;
    }

    // If the click timer is currently running and this is the same device
    // and button, this is a multiple click.
    if (click_state_.IsMultipleClick(dev, event.button))
        ++click_state_.count;
    else
        click_state_.count = 1;

    // Indicate that the device is now active.
    if (ddata.controller)
        ddata.controller->ShowActive(true, ddata.IsGrip());

    activation_time_    = Util::Time::Now();
    click_state_.device = dev;
    click_state_.button = event.button;
    click_state_.timer.Start(kClickTimeout_);
    state_ = State_::kActivated;
    KLOG('h', "MainHandler kActivated by " << Util::EnumName(event.device));

    moved_enough_for_drag_ = false;

    if (ddata.active_widget)
        ddata.active_widget->SetActive(true);
}

void MainHandler::Impl_::Deactivate_(const Event &event) {
    ASSERT(active_device_ != Device_::kNone);
    DeviceData_ &ddata = GetDeviceData_(active_device_);

    if (ddata.IsGrip() && cur_grippable_)
        cur_grippable_->ActivateGrip(ddata.controller->GetHand(), false);

    if (ddata.active_widget)
        ddata.active_widget->SetActive(false);

    if (state_ == State_::kDragging) {
        auto draggable =
            Util::CastToDerived<DraggableWidget>(ddata.active_widget);
        ASSERT(draggable);
        draggable->EndDrag();
    }

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
            ddata.hovered_widget == ddata.active_widget;

        // If the timer is not running, process the click if it is one, and
        // always reset everything.
        if (is_click)
            ProcessClick_(active_device_, event.is_alternate_mode);
        ResetClick_(event);
    }
    state_ = State_::kWaiting;
    active_device_ = Device_::kNone;
    KLOG('h', "MainHandler kWaiting after deactivation");
}

bool MainHandler::Impl_::StartOrContinueDrag_(const Event &event) {
    ASSERT(active_device_ != Device_::kNone);
    ASSERT(state_ == State_::kActivated || state_ == State_::kDragging);

    // Update the DeviceData_ for the active device.
    UpdateDeviceData_(event, active_device_, true);

    // See if this is the start of a new drag.
    const bool is_drag_start =
        state_ == State_::kActivated && ShouldStartDrag_();

    if (is_drag_start || state_ == State_::kDragging) {
        ProcessDrag_(event.is_alternate_mode);
        return true;
    }
    return false;
}

void MainHandler::Impl_::UpdateDeviceData_(const Event &event, Device_ dev,
                                           bool update_hover) {
    // 2D position is used for the mouse.
    if (event.flags.Has(Event::Flag::kPosition2D)) {
        if (dev == Device_::kNone || dev == Device_::kMouse) {
            const Ray ray = context_->frustum.BuildRay(event.position2D);
            UpdatePointerData_(event, Device_::kMouse, ray, update_hover);
        }
    }

    // 3D position + orientation is used for the controller pinch (laser
    // pointer) and grip.
    if (event.flags.Has(Event::Flag::kPosition3D) &&
        event.flags.Has(Event::Flag::kOrientation)) {
        const bool is_left = event.device == Event::Device::kLeftController;
        const Device_ pinch_dev = is_left ?
            Device_::kLeftPinch : Device_::kRightPinch;
        if (dev == Device_::kNone || dev == pinch_dev) {
            const Ray ray(event.position3D,
                          event.orientation * -Vector3f::AxisZ());
            UpdatePointerData_(event, pinch_dev, ray, update_hover);
        }

        const Device_ grip_dev = is_left ?
            Device_::kLeftGrip : Device_::kRightGrip;
        if (dev == Device_::kNone || dev == grip_dev)
            UpdateGripData_(event, grip_dev, update_hover);
    }
}

void MainHandler::Impl_::UpdatePointerData_(const Event &event, Device_ dev,
                                            const Ray &ray,
                                            bool update_hover) {
    DeviceData_ &ddata = GetDeviceData_(dev);

    WidgetPtr old_widget = ddata.hovered_widget;

    ddata.cur_ray = ray;
    ddata.cur_hit = SG::Intersector::IntersectScene(*context_->scene, ray);

    // Apply the path filter (if any) and find the lowest Widget on the path.
    if (! path_filter_ || path_filter_(ddata.cur_hit.path))
        ddata.hovered_widget = ddata.cur_hit.path.FindNodeUpwards<Widget>();
    else
        ddata.hovered_widget.reset();

    // Let the controllers know.
    if (dev == Device_::kLeftPinch || dev == Device_::kRightPinch)
        ddata.controller->ShowPointerHover(
            true, ToLocalControllerCoords(ddata.controller->GetHand(),
                                          ddata.cur_hit.point));

#if DEBUG && 0
    if (dev == Device_::kMouse) Debug::ShowHit(*context_, ddata.cur_hit);
#endif

    if (update_hover)
        UpdateWidgetHover_(old_widget, ddata.hovered_widget);
}

void MainHandler::Impl_::UpdateGripData_(const Event &event, Device_ dev,
                                         bool update_hover) {
    DeviceData_ &ddata = GetDeviceData_(dev);
    WidgetPtr old_widget = ddata.hovered_widget;

    ASSERT(ddata.controller);
    ddata.cur_ray = Ray(event.position3D,
                        ddata.controller->GetGuideDirection());

    // Start with a default constructed GripInfo.
    Grippable::GripInfo &info = ddata.cur_grip_info;
    info = Grippable::GripInfo();

    info.event = event;
    if (cur_grippable_) {
        cur_grippable_->UpdateGripInfo(info);
        ddata.hovered_widget = ddata.cur_grip_info.widget;
    }

    if (update_hover) {
        Point3f p(0, 0, 0);
        bool show = false;
        if (info.widget) {
            const auto &path = dev == Device_::kLeftGrip ?
                l_controller_path_ : r_controller_path_;
            p = path.ToLocal(cur_grippable_path_.FromLocal(info.target_point));
            show = true;
        }
        ddata.controller->ShowGripHover(show, p, info.color);
    }
}

void MainHandler::Impl_::UpdateWidgetHover_(const WidgetPtr &old_widget,
                                            const WidgetPtr &new_widget) {
    if (old_widget != new_widget ||
        (! new_widget ||  ! new_widget->IsHovering())) {
        if (old_widget)
            old_widget->SetHovering(false);
        if (new_widget)
            new_widget->SetHovering(true);
    }
}

bool MainHandler::Impl_::ShouldStartDrag_() {
    ASSERT(active_device_ != Device_::kNone);
    ASSERT(state_ == State_::kActivated);
    moved_enough_for_drag_ = MovedEnoughForDrag_();

    DeviceData_ &ddata = GetDeviceData_(active_device_);
    auto draggable =
        Util::CastToDerived<DraggableWidget>(ddata.active_widget);
    return draggable && moved_enough_for_drag_;
}

bool MainHandler::Impl_::MovedEnoughForDrag_() {
    ASSERT(active_device_ != Device_::kNone);
    DeviceData_ &ddata = GetDeviceData_(active_device_);

    const bool is_clickable = ddata.active_widget &&
        Util::CastToDerived<ClickableWidget>(ddata.active_widget);

    // If a grip drag, check for position change.
    if (ddata.IsGrip()) {
        const Point3f start_point = ddata.activation_ray.origin;
        const Point3f cur_point   = ddata.cur_ray.origin;
        if (PointMovedEnough_(start_point, cur_point, is_clickable))
            return true;
    }

    // Always do a ray-based test. Use different thresholds for pointer vs grip
    // drags.
    const Anglef threshold = (ddata.IsGrip() ? 5 : 1) * kMinRayAngle_;
    const Vector3f start_dir = ddata.activation_ray.direction;
    const Vector3f cur_dir   = ddata.cur_ray.direction;
    return DirectionMovedEnough_(start_dir, cur_dir, threshold, is_clickable);
}

void MainHandler::Impl_::ProcessDrag_(bool is_alternate_mode) {
    ASSERT(state_ == State_::kActivated || state_ == State_::kDragging);
    ASSERT(moved_enough_for_drag_);
    ASSERT(active_device_ != Device_::kNone);
    DeviceData_ &ddata = GetDeviceData_(active_device_);

    const bool is_grip_drag = ddata.IsGrip();

    // Set common items in DragInfo.
    drag_info_.is_grip = is_grip_drag;
    drag_info_.is_alternate_mode = is_alternate_mode || click_state_.count > 1;
    drag_info_.linear_precision  = precision_manager_->GetLinearPrecision();
    drag_info_.angular_precision = precision_manager_->GetAngularPrecision();

    auto draggable = Util::CastToDerived<DraggableWidget>(ddata.active_widget);
    ASSERT(draggable);

    // If starting a new drag.
    if (state_ == State_::kActivated) {
        SG::NodePath path;
        Point3f      world_pt, local_pt;
        if (is_grip_drag) {
            path = SG::FindNodePathInScene(*context_->scene, draggable);
            world_pt = ddata.activation_ray.origin;
            local_pt = path.ToLocal(world_pt);
        }
        else {
            path     = ddata.activation_hit.path;
            local_pt = ddata.activation_hit.point;
            world_pt = path.FromLocal(local_pt);
        }
        drag_info_.path        = path;
        drag_info_.world_point = world_pt;
        drag_info_.local_point = local_pt;
        drag_info_.ray         = ddata.activation_ray;

        draggable->SetHovering(false);
        draggable->StartDrag(drag_info_);

        state_ = State_::kDragging;
        KLOG('h', "MainHandler kDragging with " << draggable->GetDesc());
    }

    // Continuing a current drag operation. The path should be already set in
    // drag_info_.
    else {
        Point3f world_pt, local_pt;
        if (is_grip_drag) {
            world_pt = ddata.cur_ray.origin;
            local_pt = drag_info_.path.ToLocal(world_pt);
        }
        else {
            local_pt = ddata.cur_hit.point;
            world_pt = drag_info_.path.FromLocal(local_pt);
        }
        drag_info_.world_point = world_pt;
        drag_info_.local_point = local_pt;
        drag_info_.ray         = ddata.cur_ray;

        draggable->ContinueDrag(drag_info_);
    }
}

void MainHandler::Impl_::ProcessClick_(Device_ dev, bool is_alternate_mode) {
    ASSERT(dev != Device_::kNone);
    DeviceData_ &ddata = GetDeviceData_(dev);

    ClickInfo info;
    info.device            = ddata.event_device;
    info.hit               = ddata.cur_hit;
    info.is_alternate_mode = is_alternate_mode || click_state_.count > 1;
    info.is_long_press     =
        Util::Time::Now().SecondsSince(activation_time_) > kLongPressTime_;

    info.widget =
        Util::CastToDerived<ClickableWidget>(ddata.active_widget).get();

    clicked_.Notify(info);

    state_ = State_::kWaiting;
    KLOG('h', "MainHandler kWaiting after click on "
         << info.hit.path.ToString());
}

void MainHandler::Impl_::ResetClick_(const Event &event) {
    ASSERT(! click_state_.timer.IsRunning());

    // Indicate that the device is no longer active.
    DeviceData_ &ddata = GetDeviceData_(click_state_.device);
    if (ddata.controller)
        ddata.controller->ShowActive(false, ddata.IsGrip());

    active_device_ = Device_::kNone;
    click_state_.Reset();
}

bool MainHandler::Impl_::PointMovedEnough_(const Point3f &p0, const Point3f &p1,
                                           bool is_clickable) {
    // Use half the threshhold if the widget is not also clickable.
    const float scale = is_clickable ? 1.f : .5f;
    return ion::math::Distance(p0, p1) > scale * kMinDragDistance_;
}

bool MainHandler::Impl_::DirectionMovedEnough_(const Vector3f &d0,
                                               const Vector3f &d1,
                                               const Anglef &min,
                                               bool is_clickable) {
    using ion::math::AngleBetween;
    using ion::math::Normalized;

    // Use half the threshhold if the widget is not also clickable.
    const float scale = is_clickable ? 1.f : .5f;
    return AngleBetween(Normalized(d0), Normalized(d1)) > scale * min;
}

Device_ MainHandler::Impl_::GetDeviceForButtonEvent_(const Event &event) {
    if (event.device == Event::Device::kMouse)
        return Device_::kMouse;
    else if (event.button == Event::Button::kPinch)
        return event.device == Event::Device::kLeftController ?
            Device_::kLeftPinch : Device_::kRightPinch;
    else if (event.button == Event::Button::kGrip)
        return event.device == Event::Device::kLeftController ?
            Device_::kLeftGrip : Device_::kRightGrip;
    return Device_::kNone;
}

// ----------------------------------------------------------------------------
// MainHandler functions.
// ----------------------------------------------------------------------------

MainHandler::MainHandler() : impl_(new Impl_) {
}

MainHandler::~MainHandler() {
}

void MainHandler::SetPrecisionManager(
    const PrecisionManagerPtr &precision_manager) {
    impl_->SetPrecisionManager(precision_manager);
}

void MainHandler::SetSceneContext(const SceneContextPtr &context) {
    impl_->SetSceneContext(context);
}

void MainHandler::AddGrippable(const GrippablePtr &grippable) {
    impl_->AddGrippable(grippable);
}

Util::Notifier<const ClickInfo &> & MainHandler::GetClicked() {
    return impl_->GetClicked();
}

Util::Notifier<Event::Device, float> & MainHandler::GetValuatorChanged() {
    return impl_->GetValuatorChanged();
}

bool MainHandler::IsWaiting() const {
    return impl_->IsWaiting();
}

void MainHandler::SetPathFilter(const PathFilter &filter) {
    impl_->SetPathFilter(filter);
}

void MainHandler::ProcessUpdate(bool is_alternate_mode) {
    impl_->ProcessUpdate(is_alternate_mode);
}

bool MainHandler::HandleEvent(const Event &event) {
    return impl_->HandleEvent(event);
}

void MainHandler::Reset() {
    impl_->Reset();
}
