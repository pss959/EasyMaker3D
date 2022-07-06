#include "Handlers/MainHandler.h"

#include <vector>

#include <ion/math/vectorutils.h>

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
#include "Util/Timer.h"
#include "Util/UTime.h"
#include "Widgets/ClickableWidget.h"
#include "Widgets/DraggableWidget.h"

namespace {

// ----------------------------------------------------------------------------
// Enums.
// ----------------------------------------------------------------------------

/// Interface components that can actuate clicking or dragging.
enum class Actuator_ {
    kMouse,
    kLeftPinch,
    kRightPinch,
    kLeftGrip,
    kRightGrip,
    kLeftTouch,
    kRightTouch,
    kNone,         ///< Last so that its index is not used.
};

// ----------------------------------------------------------------------------
// MHHelper_ class.
// ----------------------------------------------------------------------------

/// MHHelper_ is an abstract base class for per-actuator helper classes for
/// MainHandler. Derived classes are implemented to hide the details of each
/// type of actuator.
class MHHelper_ {
  public:
    /// The constructor is passed the Actuator_ this helper is for, which is
    /// stored for use by derived classes.
    explicit MHHelper_(Actuator_ actuator) : actuator_(actuator) {
        ASSERT(actuator != Actuator_::kNone);
    }

    /// Sets the path filter passed to MainHandler.
    void SetPathFilter(const MainHandler::PathFilter &filter) {
        path_filter_ = filter;
    }

    /// Sets or updates the SceneContext to use.
    virtual void SetSceneContext(const SceneContextPtr &context) {
        context_ = context;
    }

    /// Returns the Widget (or null) that would be hovered by the given Event.
    /// This should also update hovering state for relevant Widgets.
    virtual WidgetPtr GetCurrentWidget(const Event &event) = 0;

    /// Saves whatever is necessary as activation data for MovedEnoughForDrag()
    /// to work.
    virtual void SaveActivationData() = 0;

    /// Returns true if the given Event represents enough motion to consider
    /// this a drag. This should be called only after SaveActivationData() is
    /// called.
    virtual bool MovedEnoughForDrag(const Event &event) = 0;

    /// Sets up the pointer-based fields or grip-based fields in the given
    /// DragInfo from the activation data.
    virtual void FillActivationDragInfo(DragInfo &info) = 0;

    /// Sets up the pointer-based fields or grip-based fields in the given
    /// DragInfo with data from the given Event.
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) = 0;

    /// Resets all state.
    virtual void Reset() = 0;

  protected:
    /// Returns the Actuator_ this helper is for.
    Actuator_ GetActuator() const { return actuator_; }

    /// Returns the SceneContext.
    SceneContext & GetContext() const {
        ASSERT(context_);
        return *context_;
    }

    /// Returns true if there is no path filter or the given Path passes it.
    bool PassesPathFilter(const SG::NodePath &path) const {
        return ! path_filter_ || path_filter_(path);
    }

    /// Convenience that updates hovering when the current Widget changes.
    void UpdateHover(const WidgetPtr &old_widget, const WidgetPtr &new_widget) {
        ASSERT(old_widget != new_widget);
        // XXXX Should really have hover count in Widget class.
        if (old_widget)
            old_widget->SetHovering(false);
        if (new_widget)
            new_widget->SetHovering(true);
    }

  private:
    Actuator_               actuator_;
    SceneContextPtr         context_;
    MainHandler::PathFilter path_filter_;
};

// ----------------------------------------------------------------------------
// MHPointerHelper_ class.
// ----------------------------------------------------------------------------

/// MHPointerHelper_ is a derived MHHelper_ class that manages pointer-based
/// actuators (mouse and pinch).
class MHPointerHelper_ : public MHHelper_ {
  public:
    explicit MHPointerHelper_(Actuator_ actuator) : MHHelper_(actuator) {}
    /// Redefines this to also store controller info.
    virtual void SetSceneContext(const SceneContextPtr &context) override;

    virtual WidgetPtr GetCurrentWidget(const Event &event) override;
    virtual void SaveActivationData() override;
    virtual bool MovedEnoughForDrag(const Event &event) override;
    virtual void FillActivationDragInfo(DragInfo &info) override;
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) override;
    virtual void Reset() override;

  private:
    Ray           current_ray_;     ///< Ray for the current Event.
    SG::Hit       current_hit_;     ///< Hit for the current Event.
    Ray           activation_ray_;  ///< Ray for the Event at activation.
    SG::Hit       activation_hit_;  ///< Hit for the Event at activation.
    WidgetPtr     hovered_widget_;  ///< From last call to GetCurrentWidget().

    ControllerPtr controller_;        ///< Controller (if pinch).
    SG::NodePath  controller_path_ ;  ///< Scene path to Controller (if pinch).

    /// If this instance is for a mouse actuator and the given Event contains
    /// mouse location info, sets Ray to the proper values and returns true.
    bool GetMouseRay_(const Event &event, Ray &ray);

    /// If this instance is for a pinch actuator and the given Event contains
    /// pinch data, sets Ray to the proper values and returns true.
    bool GetPinchRay_(const Event &event, Ray &ray);
};

void MHPointerHelper_::SetSceneContext(const SceneContextPtr &context) {
    MHHelper_::SetSceneContext(context);

    if (GetActuator() == Actuator_::kLeftPinch)
        controller_ = context->left_controller;
    else if (GetActuator() == Actuator_::kRightPinch)
        controller_ = context->right_controller;
    if (controller_)
        controller_path_ =
            SG::FindNodePathInScene(*context->scene, *controller_);
}

WidgetPtr MHPointerHelper_::GetCurrentWidget(const Event &event) {
    WidgetPtr widget;

    Ray     ray;
    SG::Hit hit;
    if (GetMouseRay_(event, ray) || GetPinchRay_(event, ray)) {
        hit = SG::Intersector::IntersectScene(*GetContext().scene, ray);
        if (PassesPathFilter(hit.path))
            widget = hit.path.FindNodeUpwards<Widget>();

        current_ray_ = ray;
        current_hit_ = hit;
    }

    if (widget != hovered_widget_) {
        UpdateHover(hovered_widget_, widget);
        hovered_widget_ = widget;

        // Update the Controller if appropriate.
        if (controller_) {
            if (hit.IsValid()) {
                const auto pt = CoordConv(controller_path_).RootToObject(
                    hit.GetWorldPoint());
                controller_->ShowPointerHover(true, pt);
            }
            else {
                controller_->ShowPointerHover(false, Point3f::Zero());
            }
        }
    }
#if DEBUG
    if (GetActuator() == Actuator_::kMouse &&
        event.device == Event::Device::kMouse) {
        if (! hit.path.empty())
            Debug::SetLimitPath(hit.path);
        Debug::DisplayText(hit.path.ToString());
    }
#endif

    return widget;
}

bool MHPointerHelper_::MovedEnoughForDrag(const Event &event) {
    /// Minimum angle between two ray directions to be considered enough for a
    // drag.
    const Anglef kMinRayAngle = Anglef::FromDegrees(2);

    // Clickable Widgets require extra motion to start a drag, since small
    // movements should not interfere with a click.
    const bool is_clickable =
        Util::CastToDerived<ClickableWidget>(hovered_widget_).get();

    // Check the ray direction change.
    const Vector3f d0 = ion::math::Normalized(activation_ray_.direction);
    const Vector3f d1 = ion::math::Normalized(current_ray_.direction);

    // Use half the threshhold if the widget is not also clickable.
    const float scale = is_clickable ? 1.f : .5f;
    return ion::math::AngleBetween(d0, d1) > scale * kMinRayAngle;
}

void MHPointerHelper_::FillActivationDragInfo(DragInfo &info) {
    info.is_grip = false;
    info.ray     = activation_ray_;
    info.hit     = activation_hit_;
}

void MHPointerHelper_::FillEventDragInfo(const Event &event, DragInfo &info) {
    Ray ray;
    if (GetMouseRay_(event, ray) || GetPinchRay_(event, ray)) {
        info.is_grip = false;
        info.ray = ray;
        info.hit = SG::Intersector::IntersectScene(*GetContext().scene, ray);
    }
}

void MHPointerHelper_::Reset() {
    activation_hit_ = current_hit_ = SG::Hit();
    hovered_widget_.reset();
}

bool MHPointerHelper_::GetMouseRay_(const Event &event, Ray &ray) {
    if (GetActuator() == Actuator_::kMouse &&
        event.device == Event::Device::kMouse &&
        event.flags.Has(Event::Flag::kPosition2D)) {
        ray = GetContext().frustum.BuildRay(event.position2D);
        return true;
    }
    return false;
}

bool MHPointerHelper_::GetPinchRay_(const Event &event, Ray &ray) {
    if (event.flags.Has(Event::Flag::kPosition3D) &&
        event.flags.Has(Event::Flag::kOrientation)) {
        if ((GetActuator() == Actuator_::kLeftPinch &&
             event.device == Event::Device::kLeftController) ||
            (GetActuator() == Actuator_::kRightPinch &&
             event.device == Event::Device::kRightController)) {
            ray = Ray(event.position3D, event.orientation * -Vector3f::AxisZ());
            return true;
        }
    }
    return false;
}

// ----------------------------------------------------------------------------
// MHGripHelper_ class.
// ----------------------------------------------------------------------------

/// MHGripHelper_ is a derived MHHelper_ class that manages grip actuators.
class MHGripHelper_ : public MHHelper_ {
  public:
    explicit MHGripHelper_(Actuator_ actuator) : MHHelper_(actuator) {}

    /// Sets the current Grippable info. If the grippable pointer is not null,
    /// the path will be from the scene root to the Grippable, allowing for
    /// coordinate conversions.
    void SetGrippable(const GrippablePtr &grippable, const SG::NodePath &path) {
        grippable_      = grippable;
        grippable_path_ = path;
    }

    /// Redefines this to also store controller info.
    virtual void SetSceneContext(const SceneContextPtr &context) override;

    virtual WidgetPtr GetCurrentWidget(const Event &event) override;
    virtual void SaveActivationData() override;
    virtual bool MovedEnoughForDrag(const Event &event) override;
    virtual void FillActivationDragInfo(DragInfo &info) override;
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) override;
    virtual void Reset() override;

  private:
    /// Stores activation or current grip data.
    struct Data_ {
        Grippable::GripInfo info;
        Point3f             position;
        Rotationf           orientation;
    };
    Data_         current_data_;     ///< Grip data for current Event.
    Data_         activation_data_;  ///< Grip data for the Event at activation.
    WidgetPtr     hovered_widget_;   ///< From last call to GetCurrentWidget().

    ControllerPtr controller_;        ///< Controller to track.
    SG::NodePath  controller_path_ ;  ///< Scene path to Controller.

    GrippablePtr  grippable_;         ///< Active Grippable (or null).
    SG::NodePath  grippable_path_;    ///< Path to active Grippable (or empty).

    bool IsGripEvent_(const Event &event) const;
};

void MHGripHelper_::SetSceneContext(const SceneContextPtr &context) {
    MHHelper_::SetSceneContext(context);

    if (GetActuator() == Actuator_::kLeftGrip)
        controller_ = context->left_controller;
    else if (GetActuator() == Actuator_::kRightGrip)
        controller_ = context->right_controller;
    ASSERT(controller_);
    controller_path_ = SG::FindNodePathInScene(*context->scene, *controller_);
}

WidgetPtr MHGripHelper_::GetCurrentWidget(const Event &event) {
    WidgetPtr widget;

    if (IsGripEvent_(event)) {
        Grippable::GripInfo &info = current_data_.info;
        info = Grippable::GripInfo();
        info.event      = event;
        info.controller = controller_;
        info.guide_direction =
            event.orientation * controller_->GetGuideDirection();

        current_data_.position    = event.position3D;
        current_data_.orientation = event.orientation;

        if (grippable_) {
            grippable_->UpdateGripInfo(info);
            hovered_widget_ = info.widget;
        }

        Point3f pt(0, 0, 0);
        const bool show = info.widget && ! grippable_path_.empty();
        if (show) {
            pt = CoordConv(controller_path_).RootToObject(
                CoordConv(grippable_path_).ObjectToRoot(info.target_point));
        }
        controller_->ShowGripHover(show, pt, info.color);
    }

    return widget;
}

bool MHGripHelper_::MovedEnoughForDrag(const Event &event) {
    /// Minimum angle between two ray directions to be considered enough for a
    // drag.
    const Anglef kMinRayAngle = Anglef::FromDegrees(10);

    /// Minimum world-space distance for a controller to move to be considered
    // a potential grip drag operation.
    const float  kMinDragDistance = .04f;

    // Clickable Widgets require extra motion to start a drag, since small
    // movements should not interfere with a click.
    const bool is_clickable =
        Util::CastToDerived<ClickableWidget>(hovered_widget_).get();
    // Use half the threshhold if the widget is not also clickable.
    const float scale = is_clickable ? 1.f : .5f;

    // Check for position change and then rotation change.
    const Point3f   &p0 = activation_data_.position;
    const Point3f   &p1 =    current_data_.position;
    const Rotationf &r0 = activation_data_.orientation;
    const Rotationf &r1 =    current_data_.orientation;
    return ion::math::Distance(p0, p1) > scale * kMinDragDistance ||
        AbsAngle(RotationAngle(RotationDifference(r0, r1))) >
        scale * kMinRayAngle;
}

void MHGripHelper_::FillActivationDragInfo(DragInfo &info) {
    info.is_grip          = true;
    info.grip_position    = activation_data_.position;
    info.grip_orientation = activation_data_.orientation;
}

void MHGripHelper_::FillEventDragInfo(const Event &event, DragInfo &info) {
    if (IsGripEvent_(event)) {
        info.is_grip          = true;
        info.grip_position    = event.position3D;
        info.grip_orientation = event.orientation;
    }
}

void MHGripHelper_::Reset() {
    current_data_.info        = Grippable::GripInfo();
    current_data_.position    = Point3f::Zero();
    current_data_.orientation = Rotationf::Identity();
    activation_data_ = current_data_;
    hovered_widget_.reset();
}

bool MHGripHelper_::IsGripEvent_(const Event &event) const {
    return
        event.flags.Has(Event::Flag::kPosition3D) &&
        event.flags.Has(Event::Flag::kOrientation) &&
        ((GetActuator() == Actuator_::kLeftGrip &&
          event.device == Event::Device::kLeftController) ||
         (GetActuator() == Actuator_::kRightGrip &&
          event.device == Event::Device::kRightController));
}

// ----------------------------------------------------------------------------
// MHTouchHelper_ class.
// ----------------------------------------------------------------------------

/// MHTouchHelper_ is a derived MHHelper_ class that manages touch actuators.
class MHTouchHelper_ : public MHHelper_ {
  public:
    explicit MHTouchHelper_(Actuator_ actuator) : MHHelper_(actuator) {}

    /// Redefines this to also store controller info.
    virtual void SetSceneContext(const SceneContextPtr &context) override;

    virtual WidgetPtr GetCurrentWidget(const Event &event) override;
    virtual void SaveActivationData() override;
    virtual bool MovedEnoughForDrag(const Event &event) override;
    virtual void FillActivationDragInfo(DragInfo &info) override;
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) override;
    virtual void Reset() override;

  private:
    ControllerPtr controller_;        ///< Controller to track.
    SG::NodePath  controller_path_ ;  ///< Scene path to Controller.
};

void MHTouchHelper_::SetSceneContext(const SceneContextPtr &context) {
    MHHelper_::SetSceneContext(context);

    if (GetActuator() == Actuator_::kLeftTouch)
        controller_ = context->left_controller;
    else if (GetActuator() == Actuator_::kRightTouch)
        controller_ = context->right_controller;
    ASSERT(controller_);
    controller_path_ = SG::FindNodePathInScene(*context->scene, *controller_);
}

WidgetPtr MHTouchHelper_::GetCurrentWidget(const Event &event) {
    WidgetPtr widget;
    // XXXX
    return widget;
}

bool MHTouchHelper_::MovedEnoughForDrag(const Event &event) {
    return false; // XXXX
}

void MHTouchHelper_::FillActivationDragInfo(DragInfo &info) {
    // XXXX
}

void MHTouchHelper_::FillEventDragInfo(const Event &event, DragInfo &info) {
    // XXXX
}

void MHTouchHelper_::Reset() {
    // XXXX
}

// ----------------------------------------------------------------------------
// ClickState_ struct.
// ----------------------------------------------------------------------------

/// ClickState_ saves information about a current potential click in progress.
struct ClickState_ {
    Timer     timer;      ///< Used to detect multiple clicks.
    int       count = 0;  ///< Current number of clicks.
    Actuator_ actuator;   ///< Actuator that started the current click.

    /// Copy of the Event that causes deactivation (once it is known).
    Event     deactivation_event;

    ClickState_() { Reset(); }

    void Reset() {
        count    = 0;
        actuator = Actuator_::kNone;
    }

    /// Returns true if the timer is currently running and the passed Actuator_
    /// matches what is stored here, meaning this is a multiple click.
    bool IsMultipleClick(Actuator_ actuator_from_event) const {
        return timer.IsRunning() && actuator_from_event == actuator;
    }
};

}  // anonymous namespace

// ----------------------------------------------------------------------------
// MainHandler::Impl_ class.
// ----------------------------------------------------------------------------

class MainHandler::Impl_ {
  public:
    Impl_() { InitHelpers_(); }
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
    void SetPathFilter(const PathFilter &filter);
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

    // ------------------------------------------------------------------------
    // Variables.

    /// Current state.
    State_ state_ = State_::kWaiting;

    /// PrecisionManager used for accessing precision details.
    PrecisionManagerPtr precision_manager_;

    /// SceneContext the handler is interacting with.
    SceneContextPtr context_;

    /// Ordered set of Grippable instances for interaction.
    std::vector<GrippablePtr> grippables_;

    /// Current Grippable: the first one that is enabled.
    GrippablePtr cur_grippable_;

    /// Notifies when a click is detected.
    Util::Notifier<const ClickInfo &> clicked_;

    /// Notifies when a valuator change is detected.
    Util::Notifier<Event::Device, float> valuator_changed_;

    /// Time at which the current device was activated.
    UTime       start_time_;

    /// Information used to detect and process clicks.
    ClickState_ click_state_;

    /// Actuator_ that caused the current activation, if any.
    Actuator_   cur_actuator_ = Actuator_::kNone;

    /// This is set to true after activation if the device moved enough to be
    /// considered a drag operation.
    bool        moved_enough_for_drag_ = false;

    /// DragInfo instance used to process drags.
    DragInfo    drag_info_;

    /// Stores events produced by touches to simulate pinches. XXXX
    std::vector<Event> extra_events_;

    /// Actuator helpers. There is one stored for each Actuator_ value except
    /// Actuator_::kNone.
    std::vector<std::unique_ptr<MHHelper_>> helpers_;

    // ------------------------------------------------------------------------
    // Functions.

    /// Used by the constructor to set up all of the helpers.
    void InitHelpers_();

    /// Returns the helper associated with an Actuator_.
    MHHelper_ & GetHelper_(Actuator_ actuator) const;

    /// Returns true if the event is a button press or touch on an enabled
    /// Widget.
    bool IsActivationEvent_(const Event &event);

    /// Processes an event that causes activation of a device.
    void ProcessActivationEvent_(const Event &event);

    /// Returns true if the given Widget (which may be null) is draggable.
    static bool IsDraggableWidget_(const WidgetPtr &widget) {
        return Util::CastToDerived<DraggableWidget>(widget).get();
    }

    /// Returns the Actuator_ corresponding to the given event, which is known
    /// to cause activation.
    static Actuator_ GetActuatorForEvent_(const Event &event);

    /// Updates both VR controllers to reflect the current state.
    void UpdateControllers_(bool is_active);

    /// Updates all pointer and grip hovering state when waiting for
    /// activation.
    void UpdateHovering_(const Event &event);

    /// XXXX
    bool IsDeactivationEvent_(const Event &event);

    /// XXXX
    void ProcessDeactivationEvent_(const Event &event);

    /// XXXX
    bool StartOrContinueDrag_(const Event &event);

#if XXXX
    // =======================================================
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
#endif

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
    void ProcessClick_(Event::Device device, bool is_alternate_mode);

    /// Resets everything after it is known that a click has finished: the
    /// timer is no longer running.
    void ResetClick_(const Event &event);

    /// Returns the Widget, if any, touched at the given 3D location.
    WidgetPtr GetTouchedWidget_(const Point3f &pt);
};

// ----------------------------------------------------------------------------
// MainHandler::Impl_ implementation.
// ----------------------------------------------------------------------------

void MainHandler::Impl_::SetSceneContext(const SceneContextPtr &context) {
    context_ = context;
    for (auto &helper: helpers_)
        helper->SetSceneContext(context);
}

void MainHandler::Impl_::SetPathFilter(const PathFilter &filter) {
    // XXXX Set path_filter_ in all pointer helpers.
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
        /* XXXX Set up grip helpers.
        if (cur_grippable_)
            cur_grippable_path_ =
                SG::FindNodePathInScene(*context_->scene, *cur_grippable_);
        */
    }

    // If the click timer finishes and not in the middle of another click or
    // drag, process the click. If not, then clear _activeData.
    if (click_state_.timer.IsFinished()) {
        if (IsWaiting()) {
            /* XXXX
            if (click_state_.count > 0)
                ProcessClick_(click_state_.actuator, is_alternate_mode);
            */
            ResetClick_(click_state_.deactivation_event);
        }
    }
}

bool MainHandler::Impl_::HandleEvent(const Event &event) {
    ASSERT(context_);

    bool handled = false;

    // Valuator events are handled specially.
    if (event.flags.Has(Event::Flag::kPosition1D)) {
        valuator_changed_.Notify(event.device, event.position1D);
        handled = true;
    }

    // If not in the middle of a click or drag.
    else if (state_ == State_::kWaiting) {
        ASSERT(cur_actuator_ == Actuator_::kNone);
        if (IsActivationEvent_(event)) {
            ProcessActivationEvent_(event);
            state_ = State_::kActivated;
            handled = true;
        }
        // If waiting for a potential end of a click, do nothing (so as not to
        // mess up the active state). Otherwise, update the hover state for all
        // devices.
        else if (! click_state_.timer.IsRunning()) {
            UpdateHovering_(event);
        }
    }

    // Either State_::kActivated or State_::kDragging.
    else {
        if (IsDeactivationEvent_(event)) {
            ProcessDeactivationEvent_(event);
            handled = true;
        }
        else {
            handled = StartOrContinueDrag_(event);
        }
    }
    return handled;
}

void MainHandler::Impl_::InitHelpers_() {
    // Shorthand.
    auto get_index = [](Actuator_ act){ return Util::EnumInt(act); };

    // Make room for all helpers.
    const int helper_count = Util::EnumCount<Actuator_>() - 1;
    ASSERT(helper_count == get_index(Actuator_::kNone));
    helpers_.resize(helper_count);

    // Shorthand
#define SET_HELPER_(act, type)                                          \
    helpers_[get_index(Actuator_::act)].reset(new type(Actuator_::act))

    SET_HELPER_(kMouse,      MHPointerHelper_);
    SET_HELPER_(kLeftPinch,  MHPointerHelper_);
    SET_HELPER_(kRightPinch, MHPointerHelper_);
    SET_HELPER_(kLeftGrip,   MHGripHelper_);
    SET_HELPER_(kRightGrip,  MHGripHelper_);
    SET_HELPER_(kLeftTouch,  MHTouchHelper_);
    SET_HELPER_(kRightTouch, MHTouchHelper_);

#undef SET_HELPER_
}

MHHelper_ & MainHandler::Impl_::GetHelper_(Actuator_ actuator) const {
    ASSERT(actuator != Actuator_::kNone);
    return *helpers_[Util::EnumInt(actuator)];
}

bool MainHandler::Impl_::IsActivationEvent_(const Event &event) {
    return event.flags.Has(Event::Flag::kButtonPress) &&
        event.button != Event::Button::kHeadset;
}

void MainHandler::Impl_::ProcessActivationEvent_(const Event &event) {
    // Determine the Actuator_ for the event.
    ASSERT(cur_actuator_ == Actuator_::kNone);
    cur_actuator_ = GetActuatorForEvent_(event);
    KLOG('h', "MainHandler kActivated by " << Util::EnumName(event.device));

    // Access the ActuatorData_ and its current Widget.
    ActuatorData_ &data = GetActuatorData_(cur_actuator_);
    active_widget_ = data.GetWidget();

    // Activate the current device data as activation data.
    data.Activate(cur_grippable_);

    // If the click timer is currently running and this is the same button,
    // this is a multiple click.
    if (click_state_.IsMultipleClick(event))
        ++click_state_.count;
    else
        click_state_.count = 1;

    // If a controller, indicate that it is now active.
    UpdateControllers_(true);

    // Set a timeout only if the click is on a Widget that is draggable.
    // Otherwise, just process the click immediately.
    const float timeout =
        IsDraggableWidget_(active_widget_) ? kClickTimeout_ : 0;

    start_time_ = UTime::Now();
    click_state_.device = event.device;
    click_state_.button = event.button;
    click_state_.timer.Start(timeout);

    moved_enough_for_drag_ = false;

    if (active_widget_)
        active_widget_->SetActive(true);
}

Actuator_ MainHandler::Impl_::GetActuatorForEvent_(const Event &event) {
    Actuator_ act = Actuator_::kNone;
    if (event.device == Event::Device::kMouse) {
        act = Actuator_::kMouse;
    }
    else if (event.device == Event::Device::kLeftController) {
        if (event.button == Event::Button::kPinch)
            act = Actuator_::kLeftPinch;
        else if (event.button == Event::Button::kGrip)
            act = Actuator_::kLeftGrip;
    }
    else if (event.device == Event::Device::kRightController) {
        if (event.button == Event::Button::kPinch)
            act = Actuator_::kRightPinch;
        else if (event.button == Event::Button::kGrip)
            act = Actuator_::kRightGrip;
    }
    ASSERT(act != Actuator_::kNone);
    return act;
}

ActuatorData_ & MainHandler::Impl_::GetActuatorData_(Actuator_ actuator) {
    switch (actuator) {
      case Actuator_::kMouse:      return mouse_data_;
      case Actuator_::kLeftPinch:  return left_pinch_data_;
      case Actuator_::kRightPinch: return right_pinch_data_;
      case Actuator_::kLeftGrip:   return left_grip_data_;
      case Actuator_::kRightGrip:  return right_grip_data_;
      case Actuator_::kLeftTouch:  return left_touch_data_;
      case Actuator_::kRightTouch: return right_touch_data_;
      default: break;
    }
    ASSERTM(false, "Invvalid actuator");
    return mouse_data_;
}

void MainHandler::Impl_::UpdateControllers_(bool is_active) {
    const auto update = [&](Controller &changed_controller,
                            Controller &other_controller,
                            bool is_now_active, bool is_grip){
        changed_controller.ShowActive(is_now_active, is_grip);
        other_controller.ShowPointer(! is_now_active);
        other_controller.ShowGrip(! is_now_active);
    };

    auto &lc = *context_->left_controller;
    auto &rc = *context_->right_controller;

    switch (cur_actuator_) {
      case Actuator_::kNone:
        ASSERTM(false, "No cur_actuator_");
        break;
      case Actuator_::kMouse:
        break;  // Nothing to do.
      case Actuator_::kLeftPinch:
        update(lc, rc, is_active, false);
        break;
      case Actuator_::kRightPinch:
        update(rc, lc, is_active, false);
        break;
      case Actuator_::kLeftGrip:
        update(lc, rc, is_active, true);
        break;
      case Actuator_::kRightGrip:
        update(rc, lc, is_active, true);
        break;

      // For touches, the controller should already be showing just the touch
      // affordance.
      case Actuator_::kLeftTouch:
        lc.Vibrate(.05f);
        break;
      case Actuator_::kRightTouch:
        rc.Vibrate(.05f);
        break;
    }
}

void MainHandler::Impl_::UpdateHovering_(const Event &event) {
    // 2D position is used for the mouse.
    if (event.flags.Has(Event::Flag::kPosition2D)) {
        if (event.device == Event::Device::kMouse) {
            const Ray ray = context_->frustum.BuildRay(event.position2D);
            UpdatePointerHover_(event, ray, mouse_data_);
        }
    }

    // 3D position + orientation is used for the controller pinch (laser
    // pointer) and grip.
    if (event.flags.Has(Event::Flag::kPosition3D) &&
        event.flags.Has(Event::Flag::kOrientation)) {
        const Ray ray(event.position3D, event.orientation * -Vector3f::AxisZ());
        if (event.device == Event::Device::kLeftController) {
            UpdatePointerHover_(event, ray, left_pinch_data_);
            UpdateGripHover_(event, left_grip_data_);
        }
        else if (event.device == Event::Device::kRightController) {
            UpdatePointerHover_(event, ray, right_pinch_data_);
            UpdateGripHover_(event, right_grip_data_);
        }
    }
}

void MainHandler::Impl_::UpdatePointerHover_(const Event &event, const Ray &ray,
                                             PointerData_ &data) {
    data.cur_data.ray = ray;
    data.cur_data.hit = SG::Intersector::IntersectScene(*context_->scene, ray);

    const auto &hit = data.cur_data.hit;

    // Apply the path filter (if any) and find the lowest Widget on the path.
    const WidgetPtr old_widget = data.hovered_widget;
    if (! path_filter_ || path_filter_(hit.path))
        data.hovered_widget = hit.path.FindNodeUpwards<Widget>();
    else
        data.hovered_widget.reset();

    // If this is a pinch update, Let the controller know.
    auto update = [&](Controller &controller){
        if (hit.IsValid())
            controller.ShowPointerHover(
                true, ToControllerCoords(controller.GetHand(),
                                         hit.GetWorldPoint()));
        else
            controller.ShowPointerHover(false, Point3f::Zero());
    };
    if (event.device == Event::Device::kLeftController)
        update(*context_->left_controller);
    else if (event.device == Event::Device::kRightController)
        update(*context_->right_controller);

#if DEBUG
    if (event.device == Event::Device::kMouse && ! hit.path.empty())
        Debug::SetLimitPath(hit.path);
#endif
#if DEBUG && 1
    if (event.device == Event::Device::kMouse)
        Debug::DisplayText(hit.path.ToString());
#endif

    UpdateWidgetHover_(old_widget, data.hovered_widget);
}

void MainHandler::Impl_::UpdateGripHover_(const Event &event, GripData_ &data) {
    ASSERT(event.device == Event::Device::kLeftController ||
           event.device == Event::Device::kRightController);
    const auto &controller = event.device == Event::Device::kLeftController ?
        context_->left_controller : context_->right_controller;

    // Set up cur_grip_info, starting with default values.
    Grippable::GripInfo &info = data.cur_data.grip_info;
    info = Grippable::GripInfo();
    info.event      = event;
    info.controller = controller;
    info.guide_direction = event.orientation * controller->GetGuideDirection();

    data.cur_data.position    = event.position3D;
    data.cur_data.orientation = event.orientation;

    if (cur_grippable_) {
        cur_grippable_->UpdateGripInfo(info);
        data.hovered_widget = data.cur_data.grip_info.widget;
    }

    Point3f pt(0, 0, 0);
    const bool show = info.widget && ! cur_grippable_path_.empty();
    if (show) {
        pt = ToControllerCoords(
            controller->GetHand(),
            CoordConv(cur_grippable_path_).ObjectToRoot(info.target_point));
    }
    controller->ShowGripHover(show, pt, info.color);
}

bool MainHandler::Impl_::IsDeactivationEvent_(const Event &event) {
        if (event.flags.Has(Event::Flag::kButtonRelease) &&
            GetDeviceForButtonEvent_(event) == active_device_) {
            Deactivate_(event);
            handled = true;
        }
}

    /// XXXX
    void ProcessDeactivationEvent_(const Event &event);

    /// XXXX
    bool StartOrContinueDrag_(const Event &event);




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
    mouse_data_.Reset();
    left_pinch_data_.Reset();
    right_pinch_data_.Reset();
    left_grip_data_.Reset();
    right_grip_data_.Reset();
    left_touch_data_.Reset();
    right_touch_data_.Reset();

    click_state_.Reset();

    state_                 = State_::kWaiting;
    cur_actuator_         = Actuator_::kNone;
    moved_enough_for_drag_ = false;

    active_widget_.reset();
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
