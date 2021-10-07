#include "MainHandler.h"

#include "Assert.h"
#include "Event.h"
#include "Math/Types.h"
#include "SG/Hit.h"
#include "SG/Intersector.h"
#include "SG/Line.h"
#include "Widgets/ClickableWidget.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/Time.h"

#include "Math/Linear.h" // XXXX For debug text.

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
    const bool is_grip;            ///< True if this represents grip data.
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

    DeviceData_(bool is_grip_in) : is_grip(is_grip_in) {}

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

// ----------------------------------------------------------------------------
// MainHandler::Impl_ class.
// ----------------------------------------------------------------------------

class MainHandler::Impl_ {
  public:
    void SetPrecisionManager(const PrecisionManagerPtr &precision_manager) {
        precision_manager_ = precision_manager;
    }
    void SetSceneContext(const SceneContextPtr &context) {
        context_ = context;
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

    /// Notifies when a click is detected.
    Util::Notifier<const ClickInfo &> clicked_;

    /// Notifies when a valuator change is detected.
    Util::Notifier<Event::Device, float> valuator_changed_;

    /// Time at which the current device was activated.
    Util::Time       activation_time_;

    /// Information used to detect and process clicks.
    ClickState_      click_state_;

    /// IGrippable instances used to manage grip interaction.
    // List<IGrippable> _grippables = new List<IGrippable>();

    /// Current IGrippable set by the last call to UpdateGrippable().
    // IGrippable    _curGrippable;

    /// \name Device Data
    /// Each of these holds the state of a tracked device.
    ///@{
    DeviceData_ mouse_data_{false};    ///< Pointer data for mouse.
    DeviceData_ l_pinch_data_{false};  ///< Pointer data for left controller.
    DeviceData_ r_pinch_data_{false};  ///< Pointer data for right controller.
    DeviceData_ l_grip_data_{true};    ///< Grip data for left controller.
    DeviceData_ r_grip_data_{true};    ///< Grip data for right controller.
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

    /// Sets _curGrippable to the current active IGrippable, which may be null.
    // void UpdateGrippable_();

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
    /// Returns null if the device is not one of the ones the MainHandler cares
    /// about.
    DeviceData_ * GetDeviceData_(const Event &event, bool is_grip);

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

    /// Returns true if the given event represents activation of a device.
    static bool IsActivationEvent_(const Event &event) {
        return event.flags.Has(Event::Flag::kButtonPress) &&
            (event.device == Event::Device::kMouse ||
             event.button == Event::Button::kPinch ||
             event.button == Event::Button::kGrip);
    }

    /// Returns true if the given event represents deactivation of a device
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

    bool handled = false;

    switch (state_) {
      case State_::kWaiting:
        // If no activation event is received, update all DeviceData_ instances
        // based on the event.
        if (IsActivationEvent_(event))
            Activate_(event);
        else
            UpdateAllDeviceData_(event);
        break;
      case State_::kActivated:
      case State_::kDragging:
        // Deal only with the active device.
        ASSERT(active_data_);
        if (event.device == active_data_->event.device) {
            UpdateDeviceData_(event, *active_data_);
            if (IsDeactivationEvent_(event, click_state_.button))
                Deactivate_();
            else if (state_ == State_::kDragging ||
                     (state_ == State_::kActivated && ShouldStartDrag_()))
                ProcessDrag_();
            handled = true;
        }
        break;
    }

    return handled;
}

void MainHandler::Impl_::Reset() {
    mouse_data_.Reset();
    l_grip_data_.Reset();
    r_grip_data_.Reset();
    l_pinch_data_.Reset();
    r_pinch_data_.Reset();

    click_state_.Reset();

    state_ = State_::kWaiting;
    active_data_ = nullptr;
    moved_enough_for_drag_ = false;
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

void MainHandler::Impl_::Activate_(const Event &event) {
    // Get the latest _DeviceData for the device and update it.
    active_data_ =
        GetDeviceData_(event, event.button == Event::Button::kGrip);
    ASSERT(active_data_);

    // Update the data from the event.
    UpdateDeviceData_(event, *active_data_);
    active_data_->activation_widget = active_data_->cur_widget;
    active_data_->activation_ray    = active_data_->cur_ray;
    active_data_->activation_hit    = active_data_->cur_hit;

    // If the click timer is currently running and this is the same device
    // and button, this is a multiple click.
    if (click_state_.IsMultipleClick(event))
        ++click_state_.count;
    else
        click_state_.count = 1;

    // XXXX _deviceManager.SetDeviceActive(ev.device as Device, true, ev);
    activation_time_    = Util::Time::Now();
    click_state_.device = event.device;
    click_state_.button = event.button;
    click_state_.timer.Start(kClickTimeout_);
    state_ = State_::kActivated;
    KLOG('h', "MainHandler kActivated by " << Util::EnumName(event.device));

    moved_enough_for_drag_ = false;
}

void MainHandler::Impl_::UpdateAllDeviceData_(const Event &event) {
    // Don't change the hovering state if waiting for the end of a click, since
    // that could mess up the active state.
    const bool update_hover = ! click_state_.timer.IsRunning();

    // Update pointer devices.
    if (event.flags.Has(Event::Flag::kPosition2D)) {
        DeviceData_ *data = GetDeviceData_(event, false);
        if (data) {
            WidgetPtr old_widget = data->cur_widget;
            UpdateDeviceData_(event, *data);
            if (update_hover) {
                UpdateHovering_(old_widget, data->cur_widget);
                // Let the device know.
                // XXXX device_manager->ShowPointerHover(device, data.point);
            }
        }
    }
    if (event.flags.Has(Event::Flag::kPosition3D)) {
        DeviceData_ *data = GetDeviceData_(event, false);
        if (data) {
            WidgetPtr old_widget = data->cur_widget;
            UpdateDeviceData_(event, *data);
            if (update_hover) {
                UpdateHovering_(old_widget, data->cur_widget);
                // Let the device know.
                // XXXX device_manager->ShowPointerHover(device, data.point);
            }
        }
    }

    // Update grip device.
    if (event.flags.Has(Event::Flag::kPosition3D)) {
#if XXXX
        DeviceData_ *data = GetDeviceData_(event, true);
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
#endif
    }
}

void MainHandler::Impl_::UpdateDeviceData_(const Event &event,
                                           DeviceData_ &data) {
    data.event = event;
    if (data.is_grip) {
        // XXXX
    }
    else {
        // Cast a window ray if there is a 2D position.
        if (event.flags.Has(Event::Flag::kPosition2D)) {
            data.cur_ray = context_->frustum.BuildRay(event.position2D);
            data.cur_hit = SG::Intersector::IntersectScene(*context_->scene,
                                                           data.cur_ray);
            data.cur_widget = data.cur_hit.path.FindNodeUpwards<Widget>();
            if (state_ != State_::kDragging) { // XXXX Debugging...
                if (data.cur_hit.IsValid()) {
                    context_->debug_sphere->SetTranslation(
                        Vector3f(data.cur_hit.point));
                    context_->debug_sphere->SetEnabled(
                        SG::Node::Flag::kRender, true);
                    context_->debug_text->SetText(data.cur_hit.path.ToString());
                }
                else {
                    context_->debug_sphere->SetEnabled(
                        SG::Node::Flag::kRender, false);
                    context_->debug_text->SetText("-");
                }
            } // XXXX End debugging...
        }
        // Cast a 3D pointer ray if there is a 3D position and orientation.
        else if (event.flags.Has(Event::Flag::kPosition3D)) {
            data.cur_ray = Ray(event.position3D,
                               event.orientation * -Vector3f::AxisZ());
            data.cur_hit = SG::Intersector::IntersectScene(*context_->scene,
                                                           data.cur_ray);
            data.cur_widget = data.cur_hit.path.FindNodeUpwards<Widget>();
        }
    }
}

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

void MainHandler::Impl_::Deactivate_() {
    ASSERT(active_data_);
    const Event &event = active_data_->event;

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
    KLOG('h', "MainHandler kWaiting after deactivation");
}

bool MainHandler::Impl_::ShouldStartDrag_() {
    ASSERT(active_data_);
    ASSERT(state_ == State_::kActivated);
    moved_enough_for_drag_ = MovedEnoughForDrag_();

    return GetDraggable_(false) && moved_enough_for_drag_;
}

bool MainHandler::Impl_::MovedEnoughForDrag_() {
    ASSERT(active_data_);

    const bool is_clickable = active_data_->activation_widget &&
        Util::CastToDerived<ClickableWidget>(active_data_->activation_widget);
    // If a grip drag, check for position change.
    /* XXXX
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
        ray_dir = active_data_->cur_hit.world_ray.direction;
    }
    return DirectionMovedEnough_(active_data_->activation_ray.direction,
                                 ray_dir, threshold, is_clickable);
}

void MainHandler::Impl_::ProcessDrag_() {
    ASSERT(state_ == State_::kActivated || state_ == State_::kDragging);
    ASSERT(active_data_);
    ASSERT(moved_enough_for_drag_);

    // Set common items in DragInfo.
    // XXXX Deal with grip drag.
    drag_info_.is_grip_drag = active_data_->is_grip;
    drag_info_.is_alternate_mode =
        active_data_->event.is_alternate_mode || click_state_.count > 1;
    drag_info_.linear_precision  = precision_manager_->GetLinearPrecision();
    drag_info_.angular_precision = precision_manager_->GetAngularPrecision();

    if (state_ == State_::kActivated) {
        // Start of a new drag.
        drag_info_.hit = active_data_->activation_hit;
        if (active_data_->activation_widget)
            active_data_->activation_widget->SetHovering(false);
        GetDraggable_()->StartDrag(drag_info_);
        state_ = State_::kDragging;
        KLOG('h', "MainHandler kDragging with " << GetDraggable_()->GetDesc());
    }
    else {
        // Continuation of current drag.
        ASSERT(state_ == State_::kDragging);
        drag_info_.hit = active_data_->cur_hit;
        GetDraggable_()->ContinueDrag(drag_info_);
    }
}

void MainHandler::Impl_::ProcessClick_(Event::Device device,
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
    KLOG('h', "MainHandler kWaiting after click on "
         << info.hit.path.ToString());
}

void MainHandler::Impl_::ResetClick_(const Event &event) {
    ASSERT(! click_state_.timer.IsRunning());
    //if (click_state_.device != Event::Device::kUnknown)
    // XXXX_deviceManager.SetDeviceActive(_clickState.device, false, event);
    active_data_ = nullptr;
    click_state_.Reset();
}

DeviceData_ * MainHandler::Impl_::GetDeviceData_(const Event &event,
                                                 bool is_grip) {
    if (event.device == Event::Device::kMouse)
        return &mouse_data_;
    else if (event.device == Event::Device::kLeftController)
        return is_grip ? &l_grip_data_ : &l_pinch_data_;
    else if (event.device == Event::Device::kRightController)
        return is_grip ? &r_grip_data_ : &r_pinch_data_;
    return nullptr;
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

Util::Notifier<const ClickInfo &> & MainHandler::GetClicked() {
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

void MainHandler::Reset() {
    impl_->Reset();
}
