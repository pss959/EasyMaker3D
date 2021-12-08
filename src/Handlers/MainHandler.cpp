#include "MainHandler.h"

#include "Debug/ShowHit.h"
#include "Event.h"
#include "GripInfo.h"
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
    Device_   device;          ///< Device this is for.
    WidgetPtr active_widget;   ///< Active Widget (or null).
    WidgetPtr hovered_widget;  ///< Widget being hovered (or null).

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
        device = Device_::kNone;
        active_widget.reset();
        hovered_widget.reset();
        activation_hit = cur_hit = SG::Hit();
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
    Event::Device device;     ///< Device that caused the current click.
    Event::Button button;     ///< Button that was pressed to start the click.

    /// Copy of the Event that causes deactivation (once it is known).
    Event         deactivation_event;

    ClickState_() { Reset(); }

    void Reset() {
        std::cerr << "XXXX Reset ClickState_\n";
        count  = 0;
        device = Event::Device::kUnknown;
        button = Event::Button::kNone;
    }

    /// Returns true if the timer is currently running and the given event uses
    /// the same device and button, meaning this is a multiple click.
    bool IsMultipleClick(Event ev) const {
        std::cerr << "XXXX IsRunning = " << timer.IsRunning()
                  << " samedev = " << (ev.device == device)
                  << " samebut = " << (ev.button == button) << "\n";
        return timer.IsRunning() &&
            ev.device == device  && ev.button == button;
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
    Device_          active_device_;

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

    /// Updates the intersection info in all relevant DeviceData_ instances
    /// based on the given event. If dev is not Device_::kNone, this restricts
    /// the update to that device. If update_hovering is true, this also
    /// updates the hovering status for all relevant Widgets.
    void UpdateIntersections_(const Event &event, Device_ dev,
                              bool update_hovering);

    /// Intersects the given ray with the scene, storing results in the given
    /// DeviceData_.
    void IntersectScene_(const Ray &ray, DeviceData_ &ddata);

    /// Updates the hovering state of the two widgets if necessary.
    void UpdateWidgetHover_(const WidgetPtr &old_widget,
                            const WidgetPtr &new_widget);

    /// Updates the grip hovering state using cur_grippable_.
    void UpdateGripHover_(const Event &event);

    /// Returns true if a drag should start.
    bool ShouldStartDrag_();

    /// Returns true if the active device indicates there was enough motion to
    /// start a drag.
    bool MovedEnoughForDrag_();

    /// Starts or continues a drag operation using the current draggable.
    void ProcessDrag_(bool is_alternate_mode);

    /// Processes a click using the given device.
    void ProcessClick_(Event::Device device, bool is_alternate_mode);

    /// Resets everything after it is known that a click has finished: the
    /// timer is no longer running.
    void ResetClick_(const Event &event);

    /// Returns the DeviceData_ for the given device.
    DeviceData_ & GetDeviceData_(Device_ dev) {
        return device_data_[Util::EnumInt(dev)];
    }

    /// Returns the active widget as a DraggableWidget.
    DraggableWidget * GetDraggable_(bool error_if_not_there = true);

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
    for (auto dev: Util::EnumValues<Device_>())
        device_data_[Util::EnumInt(dev)].device = dev;
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
    context_->debug_text->SetText(Util::EnumName(state_) + " / " +
                                  Util::EnumName(active_device_)); // XXXX
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
        if (click_state_.timer.IsRunning())
            std::cerr << "XXXX Timer Running, dev = "
                      << Util::EnumName(active_device_) << "\n";

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
        // Update the hovering state for all devices unless waiting for the end
        // of a click; that could mess up the active state.
        if (! handled && ! click_state_.timer.IsRunning())
            UpdateIntersections_(event, Device_::kNone, true);

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

    // Update the data from the event.
    UpdateIntersections_(event, active_device_, false);
    ddata.active_widget  = ddata.hovered_widget;
    ddata.activation_ray = ddata.cur_ray;
    ddata.activation_hit = ddata.cur_hit;

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

    if (ddata.active_widget)
        ddata.active_widget->SetActive(true);
}

void MainHandler::Impl_::Deactivate_(const Event &event) {
    ASSERT(active_device_ != Device_::kNone);
    DeviceData_ &ddata = GetDeviceData_(active_device_);

    if (ddata.active_widget)
        ddata.active_widget->SetActive(false);

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

    // Update the intersection info in the active DeviceData_.
    UpdateIntersections_(event, active_device_, true);

    // See if this is the start of a new drag.
    const bool is_drag_start =
        state_ == State_::kActivated && ShouldStartDrag_();

    if (is_drag_start || state_ == State_::kDragging) {
        ProcessDrag_(event.is_alternate_mode);
        return true;
    }
    return false;
}

void MainHandler::Impl_::UpdateIntersections_(const Event &event, Device_ dev,
                                              bool update_hovering) {
    // 2D position is used for the mouse.
    if (event.flags.Has(Event::Flag::kPosition2D)) {
        if (dev == Device_::kNone || dev == Device_::kMouse) {
            DeviceData_ &ddata = GetDeviceData_(Device_::kMouse);
            WidgetPtr old_widget = ddata.hovered_widget;
            const Ray ray = context_->frustum.BuildRay(event.position2D);
            IntersectScene_(ray, ddata);
#if DEBUG && 0
            Debug::ShowHit(*context_, ddata.cur_hit);
#endif
            if (update_hovering)
                UpdateWidgetHover_(old_widget, ddata.hovered_widget);
        }
    }

    // 3D position + orientation is used for the controller pinch (laser
    // pointer).
    if (event.flags.Has(Event::Flag::kPosition3D) &&
        event.flags.Has(Event::Flag::kOrientation)) {
        const Device_ edev = event.device == Event::Device::kLeftController ?
            Device_::kLeftPinch : Device_::kRightPinch;
        if (dev == Device_::kNone || dev == edev) {
            DeviceData_ &ddata = GetDeviceData_(edev);
            WidgetPtr old_widget = ddata.hovered_widget;
            const Ray ray = context_->frustum.BuildRay(event.position2D);
            IntersectScene_(ray, ddata);
            if (update_hovering) {
                UpdateWidgetHover_(old_widget, ddata.hovered_widget);
                if (cur_grippable_)
                    UpdateGripHover_(event);
            }
        }
    }
}

void MainHandler::Impl_::IntersectScene_(const Ray &ray, DeviceData_ &ddata) {
    ddata.cur_ray = ray;
    ddata.cur_hit = SG::Intersector::IntersectScene(*context_->scene, ray);

    // Apply the path filter (if any) and find the lowest Widget on the path.
    if (! path_filter_ || path_filter_(ddata.cur_hit.path))
        ddata.hovered_widget = ddata.cur_hit.path.FindNodeUpwards<Widget>();
    else
        ddata.hovered_widget.reset();

    // Let the device know.
    // XXXX device_manager->ShowPointerHover(dev, ddata.cur_hit);
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

void MainHandler::Impl_::UpdateGripHover_(const Event &event) {
    ASSERT(cur_grippable_);
    ASSERT(event.flags.Has(Event::Flag::kPosition3D) &&
           event.flags.Has(Event::Flag::kOrientation));
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

bool MainHandler::Impl_::ShouldStartDrag_() {
    ASSERT(active_device_ != Device_::kNone);
    ASSERT(state_ == State_::kActivated);
    moved_enough_for_drag_ = MovedEnoughForDrag_();

    return GetDraggable_(false) && moved_enough_for_drag_;
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

    // Set common items in DragInfo.
    drag_info_.is_grip_drag = ddata.IsGrip();
    drag_info_.is_alternate_mode = is_alternate_mode || click_state_.count > 1;
    drag_info_.linear_precision  = precision_manager_->GetLinearPrecision();
    drag_info_.angular_precision = precision_manager_->GetAngularPrecision();

    if (state_ == State_::kActivated) {
        // Start of a new drag.
        drag_info_.hit = ddata.activation_hit;
        if (ddata.active_widget)
            ddata.active_widget->SetHovering(false);
        auto draggable = GetDraggable_();
        draggable->StartDrag(drag_info_);
        state_ = State_::kDragging;
        KLOG('h', "MainHandler kDragging with " << draggable->GetDesc());
    }
    else {
        // Continuation of current drag.
        ASSERT(state_ == State_::kDragging);
        drag_info_.hit = ddata.cur_hit;
        GetDraggable_()->ContinueDrag(drag_info_);
    }
}

void MainHandler::Impl_::ProcessClick_(Device_ dev, bool is_alternate_mode) {
    ASSERT(dev != Device_::kNone);
    DeviceData_ &ddata = GetDeviceData_(dev);

    std::cerr << "XXXX Click with count = " << click_state_.count << "\n";
    ClickInfo info;
    info.device            = dev;
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
    //if (click_state_.device != Event::Device::kUnknown)
        // XXXX_deviceManager.SetDeviceActive(_clickState.device, false, event);
    active_device_ = Device_::kNone;
    click_state_.Reset();
}

DraggableWidget * MainHandler::Impl_::GetDraggable_(bool error_if_not_there) {
    ASSERT(active_device_ != Device_::kNone);
    DeviceData_ &ddata = GetDeviceData_(active_device_);
    DraggableWidget *dw =
        dynamic_cast<DraggableWidget *>(ddata.active_widget.get());
    if (error_if_not_there) {
        ASSERT(dw);
    }
    return dw;
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
