#include "MainHandler.h"

#include "Assert.h"
#include "Event.h"
#include "Math/Types.h"
#include "SG/Hit.h"
#include "SG/Intersector.h"
#include "SG/Line.h"
#include "Widgets/IDraggableWidget.h"
#include "Widgets/Widget.h"
#include "Util/General.h"
#include "Util/Time.h"

// ----------------------------------------------------------------------------
// Timer_ class.
// ----------------------------------------------------------------------------

//! Timer_ is used to check for multiple clicks within a chosen duration.
class Timer_ {
  public:
    //! Starts counting the time up to the given duration in seconds. If
    // already running, starts over.
    void Start(double duration) {
        duration_   = duration;
        start_time_ = Util::Time::Now();
    }

    //! Stops counting if it is currently counting. Does nothing if not.
    void Stop() { duration_ = 0; }

    //! Returns true if the timer is running.
    bool IsRunning() const { return duration_ > 0; }

    //! This should be called every frame to check for a finished timer. It
    // returns true if the timer was running and just hit the duration.
    bool IsFinished() {
        if (IsRunning()) {
            const double elapsed =
                Util::Time::Now().SecondsSince(start_time_);
            if (elapsed >= duration_) {
                duration_ = 0;
                return true;
            }
        }
        return false;
    }
  private:
    double     duration_ = 0;  //!< 0 when not running.
    Util::Time start_time_;
};

// ----------------------------------------------------------------------------
// DeviceData_ struct.
// ----------------------------------------------------------------------------

//! DeviceData_ saves information about a particular device, including any
//! Widget that it might be interacting with.
struct DeviceData_ {
    Event::Device device;   //!< Device this is for.
    WidgetPtr     widget;   //!< Current Widget being hovered, or null.
    Ray           ray;      //!< Ray used to find an intersection.
    Point3f       point;    //!< Intersection point.
    bool          is_grip;  //!< True if grip activation.

    DeviceData_() { Reset(); }

    //! Returns true if this instance contains valid data.
    bool IsValid() const { return device != Event::Device::kUnknown; }

    //! Resets; makes it invalid.
    void Reset() {
        device = Event::Device::kUnknown;
        widget.reset();
        is_grip = false;
    }
};

// ----------------------------------------------------------------------------
// ClickState_ struct.
// ----------------------------------------------------------------------------

//! ClickState_ saves information about a current potential click in progress.
struct ClickState_ {
    Timer_        timer;      //!< Used to detect multiple clicks.
    int           count = 0;  //!< Current number of clicks.
    Event::Device device;     //!< Device that caused the current click.
    Event::Button button;     //!< Button that was pressed to start the click.

    //! Copy of the Event that causes deactivation (once it is known).
    Event         deactivation_event;

    ClickState_() { Reset(); }

    void Reset() {
        count  = 0;
        device = Event::Device::kUnknown;
        button = Event::Button::kNone;
    }

    //! Returns true if the timer is currently running and the given event uses
    //! the same device and button, meaning this is a multiple click.
    bool IsMultipleClick(Event ev) const {
        return timer.IsRunning() &&
            ev.device == device  && ev.button == button;
    }
};

// ----------------------------------------------------------------------------
// EventPlus_ struct.
// ----------------------------------------------------------------------------

//! This is passed to a bunch of MainHandler::Impl_ functions so they have
//! access to everything they need. It wraps an Event.
struct EventPlus_ {
    Event     event;
    SG::Hit   pointer_hit;
    WidgetPtr widget;

    // XXXX Something for grip
};

// ----------------------------------------------------------------------------
// MainHandler::Impl_ class.
// ----------------------------------------------------------------------------

class MainHandler::Impl_ {
  public:
    void SetSceneContext(std::shared_ptr<SceneContext> context) {
        context_ = context;
    }
    Util::Notifier<const IClickableWidget::ClickInfo &> & GetClicked() {
        return clicked_;
    }
    Util::Notifier<Event::Device, float> & GetValuatorChanged() {
        return valuator_changed_;
    }
    bool IsWaiting() const { return state_ == State_::kWaiting; }
    void ProcessUpdate(bool is_alternate_mode);
    bool HandleEvent(const Event &event);

  private:
    // ------------------------------------------------------------------------
    // Types.

    //! Possible states.
    enum class State_ {
        kWaiting,    //!< Waiting for activation events.
        kActivated,  //!< Activation button pressed, but not dragging.
        kDragging,   //!< Activated and sufficient motion for dragging.
    };

    // ------------------------------------------------------------------------
    // Constants.

    //! Time in seconds to wait for multiple clicks.
    static constexpr float  kClickTimeout_ = .25f;

    //! Minimum time in seconds for a press to be considered a long press.
    static constexpr float  kLongPressTime_ = .6f;

    //! Minimum world-space distance for a controller to move to be considered
    // a potential grip drag operation.
    static constexpr float  kMinDragDistance_ = .04f;

    //! Minimum angle between two ray directions to be considered enough for a
    // drag.
    static const Anglef     kMinRayAngle_;

    // ------------------------------------------------------------------------
    // Variables.

    //! Current state.
    State_ state_ = State_::kWaiting;

    //!< SceneContext the handler is interacting with.
    std::shared_ptr<SceneContext> context_;

    //! Notifies when a click is detected.
    Util::Notifier<const IClickableWidget::ClickInfo &> clicked_;

    //! Notifies when a valuator change is detected.
    Util::Notifier<Event::Device, float> valuator_changed_;

    //! Time at which the current device was activated.
    Util::Time       activation_time_;

    //! Information used to detect and process clicks.
    ClickState_      click_state_;

    // ! IGrippable instances used to manage grip interaction.
    // List<IGrippable> _grippables = new List<IGrippable>();

    // ! Current IGrippable set by the last call to UpdateGrippable().
    // IGrippable    _curGrippable;

    //! \name Device Data
    //! Each of these holds the state of a tracked device.
    //!@{
    DeviceData_      mouse_data_;    //!< Pointer data for mouse.
    DeviceData_      l_pinch_data_;  //!< Pointer data for left controller.
    DeviceData_      r_pinch_data_;  //!< Pointer data for right controller.
    DeviceData_      l_grip_data_;   //!< Grip data for left controller.
    DeviceData_      r_grip_data_;   //!< Grip data for right controller.
    //!@}

    //! Points to the DeviceData_ instance for the active device, or null if no
    //! device is active.
    DeviceData_      *active_data_;

    //! This is set to true after activation if the device moved enough to be
    // considered a drag operation.
    bool             moved_enough_for_drag_ = false;

    //! DragInfo instance used to process drags.
    IDraggableWidget::DragInfo drag_info_;

    // ------------------------------------------------------------------------
    // Functions.

    //! Creates and returns an EventPlus_ instance for the given event.
    EventPlus_ CreateEventPlus_(const Event &event);

    //! Sets _curGrippable to the current active IGrippable, which may be null.
    // void UpdateGrippable_();

    //! If the given event is an activation event, activates the appropriate
    //! device. Otherwise, checks if it causes a change in hover highlight
    //! status.
    bool ActivateOrHover_(const EventPlus_ &evp);

    //! Activates the device in the given event.
    void Activate_(const EventPlus_ &evp);

    //! Updates the hover highlight for a pointing device.
    void UpdatePointerHover_(const EventPlus_ &evp);

    //! Updates the hover highlight for a controller grip.
    // void UpdateGripHover_(const EventPlus_ &evp);

    //! Updates the hovering state of the two widgets if necessary.
    static void UpdateHovering_(const WidgetPtr &old_widget,
                                const WidgetPtr &new_widget);

    //! Responds to the given event during the kActivated or kDragging states
    //! by ending a drag, deactivating the current device, or doing nothing.
    //! Returns true if dragging started.
    bool DeactivateOrDrag_(const EventPlus_ &evp);

    //! Starts a drag operation if there was enough motion.
    void StartDragIfPossible_(const EventPlus_ &evp);

    //! Returns true if an event indicates there was enough motion to start a
    //! drag.
    bool MovedEnoughForDrag_(const EventPlus_ &evp);

    //! Deactivates the current active device and finishes processing the
    //! current operation, if any, based on the given event.
    void Deactivate_(const EventPlus_ &evp);

    //! Processes a click using the given device.
    void ProcessClick_(Event::Device device, bool is_alternate_mode);

    //! Resets everything after it is known that a click has finished: the
    //! timer is no longer running.
    void ResetClick_(const Event &event);

    //! Returns a pointer to the DeviceData_ for the given device.
    DeviceData_ * GetDeviceData_(bool is_grip, Event::Device device);

    //! Returns the active widget as an IDraggableWidget.
    IDraggableWidget * GetDraggable_(bool error_if_not_there = true) {
        IDraggableWidget *dw =
            dynamic_cast<IDraggableWidget *>(active_data_->widget.get());
        if (error_if_not_there) {
            ASSERT(dw);
        }
        return dw;
    }

    //! Returns true if the two given positions are different enough to begin a
    // drag operation.
    static bool PointMovedEnough_(const Point3f &p0, const Point3f &p1,
                                  bool is_clickable) {
        // Use half the threshhold if the widget is not also clickable.
        const float scale = is_clickable ? 1.f : .5f;
        return ion::math::Distance(p0, p1) > scale * kMinDragDistance_;
    }

    //! Returns true if the two given directions are different enough to
    // begin a drag operation.
    static bool DirectionMovedEnough_(const Vector3f &d0, const Vector3f &d1,
                                      const Anglef &min, bool is_clickable) {
        // Use half the threshhold if the widget is not also clickable.
        const float scale = is_clickable ? 1.f : .5f;
        return ion::math::AngleBetween(d0, d1) > scale * min;
    }

    //! Returns true if the given event represents activation of a device.
    static bool IsActivationEvent_(const Event &event) {
        return event.flags.Has(Event::Flag::kButtonPress) &&
            (event.device == Event::Device::kMouse ||
             event.button == Event::Button::kPinch ||
             event.button == Event::Button::kGrip);
    }

    //! Returns true if the given event represents deactivation of a device
    // with the given button.
    static bool IsDeactivationEvent_(const Event &event, Event::Button button) {
        return event.flags.Has(Event::Flag::kButtonRelease) &&
            event.button == button;
    }
};

const Anglef MainHandler::Impl_::kMinRayAngle_ = Anglef::FromDegrees(2);

// ----------------------------------------------------------------------------
// MainHandler::Impl_ implementation.
// ----------------------------------------------------------------------------

void MainHandler::Impl_::ProcessUpdate(bool is_alternate_mode) {
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

    // Handle valuator events first.
    if (event.flags.Has(Event::Flag::kPosition1D))
        valuator_changed_.Notify(event.device, event.position1D);

    // Set up an EventPlus_ to pass to other functions.
    EventPlus_ evp = CreateEventPlus_(event);

    bool handled = false;

    switch (state_) {
      case State_::kWaiting:
        handled = ActivateOrHover_(evp);
        break;
      case State_::kActivated:
        if (event.device == active_data_->device)
            handled = DeactivateOrDrag_(evp);
        break;
      case State_::kDragging:
        if (event.device == active_data_->device)
            handled = DeactivateOrDrag_(evp);
        break;
    }

    return handled;
}

EventPlus_ MainHandler::Impl_::CreateEventPlus_(const Event &event) {
    EventPlus_ evp;
    evp.event = event;

    // Cast a ray if there is a 2D position.
    if (event.flags.Has(Event::Flag::kPosition2D)) {
        const Ray ray = context_->frustum.BuildRay(event.position2D);
        evp.pointer_hit =
            SG::Intersector::IntersectScene(*context_->scene, ray);
        evp.widget = evp.pointer_hit.path.FindNodeUpwards<Widget>();

        if (evp.pointer_hit.IsValid()) {
            context_->debug_sphere->SetTranslation(
                Vector3f(evp.pointer_hit.point));
            context_->debug_sphere->SetEnabled(SG::Node::Flag::kRender, true);
        }
        else {
            context_->debug_sphere->SetEnabled(SG::Node::Flag::kRender, false);
        }
    }

    // Do something with grip.
    // XXXX

    return evp;
}

#if XXXX
void MainHandler::Impl_::UpdateGrippable_() {
    _curGrippable = _grippables.Find((g) => g.IsGrippableEnabled());

    // Update the grip guide.
    _deviceManager.SetGripGuideType(
        _curGrippable == null ? GripGuide.GuideType.None :
        _curGrippable.GetGripGuideType());
}
#endif

bool MainHandler::Impl_::ActivateOrHover_(const EventPlus_ &evp) {
    bool handled = true;
    if (IsActivationEvent_(evp.event)) {
        Activate_(evp);
    }
    // Don't hover if waiting for the end of a click, since that could mess up
    // the active state.
    else if (! click_state_.timer.IsRunning()) {
        if (evp.event.flags.Has(Event::Flag::kPosition2D))
            UpdatePointerHover_(evp);
        /* XXXX
        if (ev.flags.HasFlag(DEvent.Flag.GripData))
            GripHover(ev);
        */
        else
            handled = false;
    }
    return handled;
}

void MainHandler::Impl_::Activate_(const EventPlus_ &evp) {
    // Get the latest _DeviceData for the device and update it.
    active_data_ = GetDeviceData_(evp.event.button == Event::Button::kGrip,
                                  evp.event.device);
    ASSERT(active_data_ && active_data_->IsValid());

    // Update the rays from the new event.
    if (active_data_->is_grip) {
        /* XXXX
        ASSERT(event.flags.Has(Event::Flag::kGripData));
        _activeData.ray = ev.gripData.GetRay();
        */
    }
    else {
        active_data_->ray = evp.pointer_hit.world_ray;
    }

    // If the click timer is currently running and this is the same device
    // and button, this is a multiple click.
    if (click_state_.IsMultipleClick(evp.event))
        ++click_state_.count;
    else
        click_state_.count = 1;

    // XXXX _deviceManager.SetDeviceActive(ev.device as Device, true, ev);
    activation_time_    = Util::Time::Now();
    click_state_.device = evp.event.device;
    click_state_.button = evp.event.button;
    click_state_.timer.Start(kClickTimeout_);
    state_ = State_::kActivated;

    moved_enough_for_drag_ = false;
}

void MainHandler::Impl_::UpdatePointerHover_(const EventPlus_ &evp) {
    // Get the current DeviceData_ or create one.
    DeviceData_ *data = GetDeviceData_(false, evp.event.device);
    WidgetPtr  old_widget = data->widget;

    data->widget  = evp.widget;
    data->point   = evp.pointer_hit.point;
    data->device  = evp.event.device;
    data->is_grip = false;

    UpdateHovering_(old_widget, data->widget);

    // Let the device know.
    // XXXX device_manager->ShowPointerHover(device, data.point);
}

#if XXXX
void MainHandler::Impl_::UpdateGripHover_(XXXX) {
    Device dev = ev.device as Device;

    // Get the current _DeviceData or create one.
    _DeviceData data = GetDeviceData_(true, dev);
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
    dev.ShowGripHover(data.hadGO, data.point, ev.gripData.color);
}
#endif

void MainHandler::Impl_::UpdateHovering_(const WidgetPtr &old_widget,
                                         const WidgetPtr &new_widget) {
    if (old_widget != new_widget ||
        (! new_widget ||  ! new_widget->IsHovering())) {
        if (old_widget)
            old_widget->SetHovering(false);
        if (new_widget)
            new_widget->SetHovering(true);
    }
}

bool MainHandler::Impl_::DeactivateOrDrag_(const EventPlus_ &evp) {
    ASSERT(state_ == State_::kActivated || state_ == State_::kDragging);
    bool handled = true;
    if (IsDeactivationEvent_(evp.event, click_state_.button)) {
        if (state_ == State_::kDragging)
            GetDraggable_()->EndDrag();
        Deactivate_(evp);
    }
    else {
        if (state_ == State_::kDragging) {
            drag_info_.hit = evp.pointer_hit;
            drag_info_.is_alternate_mode =
                evp.event.is_alternate_mode || click_state_.count > 1;
            GetDraggable_()->Drag(drag_info_);
        }
        else {
            StartDragIfPossible_(evp);
            handled = state_ == State_::kDragging;
        }
    }
    return handled;
}

void MainHandler::Impl_::StartDragIfPossible_(const EventPlus_ &evp) {
    ASSERT(active_data_ && active_data_->IsValid());

    moved_enough_for_drag_ = MovedEnoughForDrag_(evp);

    IDraggableWidget *draggable = GetDraggable_(false);
    if (draggable && moved_enough_for_drag_) {
        // XXXX Deal with grip drag.
        drag_info_.hit          = evp.pointer_hit;
        drag_info_.is_grip_drag = active_data_->is_grip;
        drag_info_.is_alternate_mode =
            evp.event.is_alternate_mode || click_state_.count > 1;
        if (active_data_->widget)
            active_data_->widget->SetHovering(false);

        draggable->StartDrag(drag_info_);
        state_ = State_::kDragging;
    }
}

bool MainHandler::Impl_::MovedEnoughForDrag_(const EventPlus_ &evp) {
    const bool is_clickable = false; // XXXX
    /* XXXX
    const bool isClickable = active_data_->widget &&
        active_data_-> .GetComponent<IClickable>() != null;
    // If a grip drag, check for position change.
    if (active_data_->isGrip &&
        PointMovedEnough_(active_data_->ray.origin, ev.gripData.position,
        is_clickable))
        return true;
    */

    // Always do a ray-based test. Use different thresholds for pointer vs
    // grip drags.
    Vector3f ray_dir;
    Anglef   threshold = kMinRayAngle_;
    if (active_data_->is_grip) {
        /* XXXX
        rayDir = ev.gripData.direction;
        threshold *= 5f;  // Need more rotation for grip drags.
        */
    }
    else {
        ray_dir = evp.pointer_hit.world_ray.direction;
    }
    return DirectionMovedEnough_(active_data_->ray.direction, ray_dir,
                                 threshold, is_clickable);
}

//! Deactivates the current active device and finishes processing the
// current operation, if any.
void MainHandler::Impl_::Deactivate_(const EventPlus_ &evp) {
    if (click_state_.timer.IsRunning()) {
        // If the timer is still running, save the deactivation event.
        click_state_.deactivation_event = evp.event;
    }
    else {
        // The deactivation represents a click if all of the following are
        // true:
        //   - A drag is not in process.
        //   - The active device did not move too much (enough for a drag).
        //   - The active device did not move off the clickable widget.
        const bool is_click = state_ != State_::kDragging &&
            ! moved_enough_for_drag_ && evp.widget == active_data_->widget;

        // If the timer is not running, process the click if it is one, and
        // always reset everything.
        if (is_click)
            ProcessClick_(evp.event.device, evp.event.is_alternate_mode);
        ResetClick_(evp.event);
    }
    state_ = State_::kWaiting;
}

//! Processes a click using the given device.
void MainHandler::Impl_::ProcessClick_(Event::Device device,
                                       bool is_alternate_mode) {
    /* XXXX
    ASSERT(active_data_->IsValid());

    ClickInfo info = new ClickInfo();
    info.device          = dev;
    info.go              = active_data_->go;
    info.worldPoint      = active_data_->point;
    info.isAlternateMode = isAlternateMode || _clickState.count > 1;
    // Check for long press.
    double pressTime = (DateTime.Now - _activationTime).TotalSeconds;
    info.isLongPress = pressTime >= _LongPressTime;

    // Access the IClickable from the GameObject and allow it to act as a
    // stand-in.
    IClickable clickable = null;
    if (info.go != null)
        clickable = info.go.GetComponent<IClickable>()?.GetClickable();
    Clicked.Invoke(clickable, info);
    */

    state_ = State_::kWaiting;
}

//! Resets everything after it is known that a click has finished: the
// timer is no longer running.
void MainHandler::Impl_::ResetClick_(const Event &event) {
    ASSERT(! click_state_.timer.IsRunning());
    //if (click_state_.device != Event::Device::kUnknown)
    // XXXX_deviceManager.SetDeviceActive(_clickState.device, false, event);
    active_data_ = nullptr;
    click_state_.Reset();
}

//! Returns the current _DeviceData from the dictionary for the type of
// activation for the given Device, creating it if necessary.
DeviceData_ * MainHandler::Impl_::GetDeviceData_(bool is_grip,
                                                 Event::Device device) {
    if (device == Event::Device::kMouse)
        return &mouse_data_;
    else if (device == Event::Device::kLeftController)
        return is_grip ? &l_grip_data_ : &l_pinch_data_;
    else if (device == Event::Device::kRightController)
        return is_grip ? &r_grip_data_ : &r_pinch_data_;
    ASSERTM(false, "Unknown device passed to GetDeviceData_()");
    return nullptr;
}

// ----------------------------------------------------------------------------
// MainHandler functions.
// ----------------------------------------------------------------------------

MainHandler::MainHandler() : impl_(new Impl_) {
}

MainHandler::~MainHandler() {
}

void MainHandler::SetSceneContext(std::shared_ptr<SceneContext> context) {
    impl_->SetSceneContext(context);
}

Util::Notifier<const IClickableWidget::ClickInfo &> &
MainHandler::GetClicked() {
    return impl_->GetClicked();
}

Util::Notifier<Event::Device, float> & MainHandler::GetValuatorChanged() {
    return impl_->GetValuatorChanged();
}

bool MainHandler::IsWaiting() const {
    return impl_->IsWaiting();
}

void MainHandler::ProcessUpdate(bool is_alternate_mode) {
    impl_->ProcessUpdate(is_alternate_mode);
}

bool MainHandler::HandleEvent(const Event &event) {
    return impl_->HandleEvent(event);
}
