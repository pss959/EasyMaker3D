#include "Handlers/MainHandler.h"

#include "App/CoordConv.h"
#include "App/DragInfo.h"
#include "App/SceneContext.h"
#include "Base/Event.h"
#include "Debug/Print.h"
#include "Items/Controller.h"
#include "Items/Grippable.h"
#include "Managers/PrecisionManager.h"
#include "Math/Types.h"
#include "SG/Hit.h"
#include "SG/Intersector.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/UTime.h"
#include "Widgets/ClickableWidget.h"
#include "Widgets/DraggableWidget.h"

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
        start_time_ = UTime::Now();
    }

    /// Stops counting if it is currently counting. Does nothing if not.
    void Stop() { duration_ = 0; }

    /// Returns true if the timer is running.
    bool IsRunning() const { return duration_ > 0; }

    /// This should be called every frame to check for a finished timer. It
    // returns true if the timer was running and just hit the duration.
    bool IsFinished() {
        if (IsRunning() &&
            UTime::Now().SecondsSince(start_time_) >= duration_) {
            duration_ = 0;
            return true;
        }
        return false;
    }

  private:
    double duration_ = 0;  ///< Set to 0 when not running.
    UTime  start_time_;
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
    /// Widget last hovered by this Device_ (may be null).
    WidgetPtr     hovered_widget;
    /// Widget in active use by this Device_ (may be null).
    WidgetPtr     active_widget;
    /// Widget last intersected by a touch event. Only for kLeftPinch or
    /// kRightPinch, and usually null.
    WidgetPtr     touched_widget;

    /// \name Pointer data
    /// These items are stored only for pointer devices (mouse and pinch).
    ///@{
    Ray     start_ray;  ///< Ray at activation.
    SG::Hit start_hit;  ///< Intersection info for start_ray.
    Ray     cur_ray;    ///< Current ray.
    SG::Hit cur_hit;    ///< Intersection info for cur_ray.
    ///@}

    /// \name Grip data
    /// These items are stored only for grip devices. The controller position
    /// and orientation (in world coordinates) are both stored.
    ///@{
    Grippable::GripInfo start_grip_info;    ///< Grip info at activation.
    Point3f             start_position;     ///< Position at activation.
    Rotationf           start_orientation;  ///< Orientation at activation.
    Grippable::GripInfo cur_grip_info;      ///< Current grip info.
    Point3f             cur_position;       ///< Current position.
    Rotationf           cur_orientation;    ///< Current orientation.
    ///@}

    /// Resets to default state.
    void Reset() {
        active_widget.reset();
        hovered_widget.reset();
        start_hit         = cur_hit         = SG::Hit();
        start_grip_info   = cur_grip_info   = Grippable::GripInfo();
        start_position    = cur_position    = Point3f::Zero();
        start_orientation = cur_orientation = Rotationf::Identity();
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
    Event         destart_event;

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
    void SetSceneContext(const SceneContextPtr &context);
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
    std::vector<Event> GetExtraEvents();
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
    /// controller coordinates.
    SG::NodePath l_controller_path_;
    /// Path from the scene root to the right controller, used to convert to
    /// controller coordinates.
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
    UTime       start_time_;

    /// Information used to detect and process clicks.
    ClickState_ click_state_;

    /// Function passed to SetPathFilter().
    PathFilter  path_filter_;

    /// DeviceData_ for each Device_ (including Device_::kNone for simplicity).
    DeviceData_ device_data_[Util::EnumCount<Device_>()];

    /// Device_ that is currently active, possibly Device_::kNone.
    Device_     active_device_ = Device_::kNone;

    /// This is set to true after activation if the device moved enough to be
    /// considered a drag operation.
    bool        moved_enough_for_drag_ = false;

    /// DragInfo instance used to process drags.
    DragInfo    drag_info_;

    /// Stores events produced by touches to simulate pinches.
    std::vector<Event> extra_events_;

    // ------------------------------------------------------------------------
    // Functions.

    /// Handles valuator events.
    bool HandleValuatorEvent_(const Event &event);
    /// Converts touch events into pinch events.
    bool HandleTouchEvent_(const Event &event);
    /// Handles all other click- or drag-related events.
    bool HandleClickOrDragEvent_(const Event &event);

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

    /// Returns the Widget, if any, touched at the given 3D location.
    WidgetPtr GetTouchedWidget_(const Point3f &pt);

    /// Returns the DeviceData_ for the given device.
    DeviceData_ & GetDeviceData_(Device_ dev) {
        return device_data_[Util::EnumInt(dev)];
    }

    /// Converts a point from world coordinates into object coordinates for the
    /// controller with the given Hand.
    Point3f ToControllerCoords(Hand hand, const Point3f &p) {
        const auto &path =
            hand == Hand::kLeft ? l_controller_path_ : r_controller_path_;
        return CoordConv(path).RootToObject(p);
    }

    /// Returns true if the two given positions are different enough to begin a
    /// drag operation.
    static bool PointMovedEnough_(const Point3f &p0, const Point3f &p1,
                                  bool is_clickable);
    /// Returns true if the two given directions are different enough to
    /// begin a drag operation.
    static bool DirectionMovedEnough_(const Vector3f &d0, const Vector3f &d1,
                                      const Anglef &min, bool is_clickable);
    /// Returns true if the two given orientations are different enough to
    /// begin a drag operation.
    static bool RotationMovedEnough_(const Rotationf &r0, const Rotationf &r1,
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

void MainHandler::Impl_::SetSceneContext(const SceneContextPtr &context) {
    context_ = context;

    // Skip the rest if there are no controllers in the scene.
    if (! context_->left_controller)
        return;

    // Save paths to the controllers.
    l_controller_path_ =
        SG::FindNodePathInScene(*context_->scene, *context_->left_controller);
    r_controller_path_ =
        SG::FindNodePathInScene(*context_->scene, *context_->right_controller);
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
                SG::FindNodePathInScene(*context_->scene, *cur_grippable_);
    }

    // If the click timer finishes and not in the middle of another click or
    // drag, process the click. If not, then clear _activeData.
    if (click_state_.timer.IsFinished()) {
        if (IsWaiting()) {
            if (click_state_.count > 0)
                ProcessClick_(click_state_.device, is_alternate_mode);
            ResetClick_(click_state_.destart_event);
        }
    }
}

bool MainHandler::Impl_::HandleEvent(const Event &event) {
    if (! context_)
        return false;

    return
        HandleValuatorEvent_(event) ||
        HandleTouchEvent_(event) ||
        HandleClickOrDragEvent_(event);
}

bool MainHandler::Impl_::HandleValuatorEvent_(const Event &event) {
    if (event.flags.Has(Event::Flag::kPosition1D)) {
        valuator_changed_.Notify(event.device, event.position1D);
        return true;
    }
    return false;
}

bool MainHandler::Impl_::HandleTouchEvent_(const Event &event) {
    bool handled = false;

    // Turn touch events on widgets into pinch events.
    if (event.flags.Has(Event::Flag::kTouch)) {
        auto dev = event.device == Event::Device::kLeftController ?
            Device_::kLeftPinch : Device_::kRightPinch;
        DeviceData_ &ddata = GetDeviceData_(dev);
        auto widget = GetTouchedWidget_(event.touch_position3D);
        if (widget != ddata.touched_widget) {
            // Set up an Event to simulate a pinch.
            Event ev;
            ev.device = event.device;
            ev.flags.Set(Event::Flag::kPosition3D);
            ev.flags.Set(Event::Flag::kOrientation);
            ev.button = Event::Button::kPinch;
            ev.position3D = event.touch_position3D;

            // If no longer touching a widget, release it.
            if (ddata.touched_widget) {
                ev.flags.Set(Event::Flag::kButtonRelease);
                extra_events_.push_back(ev);
            }

            // If now touching a widget, pinch it.
            if (widget) {
                ev.flags.Reset(Event::Flag::kButtonRelease);
                ev.flags.Set(Event::Flag::kButtonPress);
                extra_events_.push_back(ev);
            }
            ddata.touched_widget = widget;
            handled = true;

            ddata.controller->Vibrate(.25f);
        }
        else {
            handled = widget.get();
        }
    }
    return handled;
}

bool MainHandler::Impl_::HandleClickOrDragEvent_(const Event &event) {
    bool handled = false;

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

std::vector<Event> MainHandler::Impl_::GetExtraEvents() {
    if (! extra_events_.empty()) {
        auto copy = extra_events_;
        extra_events_.clear();
        return copy;
    }
    else {
        return extra_events_;
    }
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

    // Update the data for the active device from the event.  Since this is an
    // activation, copy the current DeviceData_ information to the start_
    // fields.
    UpdateDeviceData_(event, active_device_, false);
    ddata.active_widget = ddata.hovered_widget;
    if (ddata.IsGrip()) {
        ddata.start_grip_info    = ddata.cur_grip_info;
        ddata.start_position     = ddata.cur_position;
        ddata.start_orientation  = ddata.cur_orientation;
        // Also activate the grip.
        if (cur_grippable_)
            cur_grippable_->ActivateGrip(ddata.controller->GetHand(), true);
    }
    else {
        ddata.start_ray = ddata.cur_ray;
        ddata.start_hit = ddata.cur_hit;
    }

    // If the click timer is currently running and this is the same device
    // and button, this is a multiple click.
    if (click_state_.IsMultipleClick(dev, event.button))
        ++click_state_.count;
    else
        click_state_.count = 1;

    // Indicate that the device is now active.
    if (ddata.controller) {
        ddata.controller->ShowActive(true, ddata.IsGrip());
        auto &other_controller =
            ddata.controller->GetHand() == Hand::kLeft ?
            context_->right_controller : context_->left_controller;
        other_controller->ShowPointer(false);
        other_controller->ShowGrip(false);
    }

    // Set a timeout only if the click is on a Widget that is draggable.
    // Otherwise, just process the click immediately.
    const float timeout =
        Util::CastToDerived<DraggableWidget>(ddata.active_widget) ?
        kClickTimeout_ : 0;

    start_time_ = UTime::Now();
    click_state_.device = dev;
    click_state_.button = event.button;
    click_state_.timer.Start(timeout);
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
        click_state_.destart_event = event;
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
    const bool update_hover = state_ != State_::kDragging;
    UpdateDeviceData_(event, active_device_, update_hover);

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
    if (dev == Device_::kLeftPinch || dev == Device_::kRightPinch) {
        if (ddata.cur_hit.IsValid())
            ddata.controller->ShowPointerHover(
                true, ToControllerCoords(ddata.controller->GetHand(),
                                         ddata.cur_hit.GetWorldPoint()));
        else
            ddata.controller->ShowPointerHover(false, Point3f::Zero());
    }

#if DEBUG
    if (dev == Device_::kMouse && ! ddata.cur_hit.path.empty())
        Debug::SetLimitPath(ddata.cur_hit.path);
#endif
#if DEBUG && 0
    if (dev == Device_::kMouse)
        Debug::DisplayText(ddata.cur_hit.path.ToString());
#endif

    if (update_hover)
        UpdateWidgetHover_(old_widget, ddata.hovered_widget);
}

void MainHandler::Impl_::UpdateGripData_(const Event &event, Device_ dev,
                                         bool update_hover) {
    DeviceData_ &ddata = GetDeviceData_(dev);
    ASSERT(ddata.controller);
    ASSERT(event.flags.Has(Event::Flag::kPosition3D));
    ASSERT(event.flags.Has(Event::Flag::kOrientation));

    // Set up cur_grip_info, starting with default values.
    Grippable::GripInfo &info = ddata.cur_grip_info;
    info = Grippable::GripInfo();
    info.event      = event;
    info.controller = ddata.controller;
    info.guide_direction =
        event.orientation * info.controller->GetGuideDirection();

    ddata.cur_position    = event.position3D;
    ddata.cur_orientation = event.orientation;

    if (cur_grippable_) {
        cur_grippable_->UpdateGripInfo(info);
        ddata.hovered_widget = ddata.cur_grip_info.widget;
    }

    // XXXX Always update_hover; remove parameter.

    Point3f p(0, 0, 0);
    const bool show = info.widget && ! cur_grippable_path_.empty();
    if (show) {
        p = ToControllerCoords(
            ddata.controller->GetHand(),
            CoordConv(cur_grippable_path_).ObjectToRoot(info.target_point));
    }
    ddata.controller->ShowGripHover(show, p, info.color);
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

    // Clickable Widgets require extra motion to start a drag, since small
    // movements should not interfere with a click.
    const bool is_clickable = ddata.active_widget &&
        Util::CastToDerived<ClickableWidget>(ddata.active_widget);

    // If a grip drag, check for position and orientation changes.  Use a
    // larger rotation threshold for grip drags.
    if (ddata.IsGrip()) {
        return PointMovedEnough_(ddata.start_position, ddata.cur_position,
                                 is_clickable) ||
            RotationMovedEnough_(ddata.start_orientation,
                                 ddata.cur_orientation, 5 * kMinRayAngle_,
                                 is_clickable);
    }
    // Otherwise, just do a ray-based test.
    else {
        return DirectionMovedEnough_(ddata.start_ray.direction,
                                     ddata.cur_ray.direction, kMinRayAngle_,
                                     is_clickable);
    }
}

void MainHandler::Impl_::ProcessDrag_(bool is_alternate_mode) {
    ASSERT(state_ == State_::kActivated || state_ == State_::kDragging);
    ASSERT(moved_enough_for_drag_);
    ASSERT(active_device_ != Device_::kNone);
    DeviceData_ &ddata = GetDeviceData_(active_device_);

    const bool is_grip_drag = ddata.IsGrip();

    // Set common items in DragInfo.
    drag_info_.is_grip           = is_grip_drag;
    drag_info_.is_alternate_mode = is_alternate_mode || click_state_.count > 1;
    drag_info_.linear_precision  = precision_manager_->GetLinearPrecision();
    drag_info_.angular_precision = precision_manager_->GetAngularPrecision();
    drag_info_.path_to_stage     = context_->path_to_stage;

    auto draggable = Util::CastToDerived<DraggableWidget>(ddata.active_widget);
    ASSERT(draggable);

    // If starting a new drag.
    if (state_ == State_::kActivated) {
        // This is set once and used throughout the drag.
        drag_info_.path_to_widget =
            SG::FindNodePathInScene(*context_->scene, *draggable);

        // Set drag-specific items.
        if (is_grip_drag) {
            drag_info_.grip_position    = ddata.start_position;
            drag_info_.grip_orientation = ddata.start_orientation;
        }
        else {
            drag_info_.ray = ddata.start_ray;
            drag_info_.hit = ddata.start_hit;
        }

        draggable->SetHovering(false);
        draggable->StartDrag(drag_info_);

        state_ = State_::kDragging;
        KLOG('h', "MainHandler kDragging with " << draggable->GetDesc()
             << " (" << drag_info_.path_to_widget.ToString() << ")");
    }
    // Continuing a current drag operation.
    else {
        // Set drag-specific items.
        if (is_grip_drag) {
            drag_info_.grip_position    = ddata.cur_position;
            drag_info_.grip_orientation = ddata.cur_orientation;
        }
        else {
            drag_info_.ray = ddata.cur_ray;
            drag_info_.hit = ddata.cur_hit;
        }
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
        UTime::Now().SecondsSince(start_time_) > kLongPressTime_;

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
    if (ddata.controller) {
        ddata.controller->ShowActive(false, ddata.IsGrip());
        auto &other_controller =
            ddata.controller->GetHand() == Hand::kLeft ?
            context_->right_controller : context_->left_controller;
        other_controller->ShowPointer(true);
        other_controller->ShowGrip(true);
    }

    active_device_ = Device_::kNone;
    click_state_.Reset();
}

WidgetPtr MainHandler::Impl_::GetTouchedWidget_(const Point3f &pt) {
    WidgetPtr widget;
    Ray ray(pt, -Vector3f::AxisZ());
    SG::Hit hit = SG::Intersector::IntersectScene(*context_->scene, ray);
    if (hit.IsValid()) {
        // The touch affordance has to be within the kTouchRadius in front or a
        // multiple of the radius behind the widget (allowing for piercing to
        // work).
        const float dist = hit.distance;
        if ((dist >= 0 && dist <= Defaults::kTouchRadius) ||
            (dist <  0 && dist >= -4 * Defaults::kTouchRadius))
            widget = hit.path.FindNodeUpwards<Widget>();
    }

    return widget;
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

bool MainHandler::Impl_::RotationMovedEnough_(const Rotationf &r0,
                                              const Rotationf &r1,
                                              const Anglef &min,
                                              bool is_clickable) {
    // Use half the threshhold if the widget is not also clickable.
    const float scale = is_clickable ? 1.f : .5f;
    return AbsAngle(RotationAngle(RotationDifference(r0, r1))) > scale * min;
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

std::vector<Event> MainHandler::GetExtraEvents() {
    return impl_->GetExtraEvents();
}

void MainHandler::Reset() {
    impl_->Reset();
}
