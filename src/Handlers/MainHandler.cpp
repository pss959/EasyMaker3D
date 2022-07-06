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
    virtual WidgetPtr GetWidgetForEvent(const Event &event) = 0;

    /// Returns the last Widget returned by GetWidgetForEvent().
    virtual WidgetPtr GetCurrentWidget() const = 0;

    /// Does whatever is necessary for activation or deactivation. When
    /// activating, also saves whatever is necessary as activation data for
    /// MovedEnoughForDrag() to work.
    virtual void SetActive(bool is_active) = 0;

    /// Returns true if the given Event represents enough motion to consider
    /// this a drag. This should be called only after SetActive(true) is
    /// called.
    virtual bool MovedEnoughForDrag(const Event &event) = 0;

    /// Sets up the pointer-based fields or grip-based fields in the given
    /// DragInfo from the activation data.
    virtual void FillActivationDragInfo(DragInfo &info) = 0;

    /// Sets up the pointer-based fields or grip-based fields in the given
    /// DragInfo with data from the given Event.
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) = 0;

    /// Sets up the actuator-dependent fields in the given ClickInfo with the
    /// current state from the helper.
    virtual void FillClickInfo(ClickInfo &info) = 0;

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

    virtual WidgetPtr GetWidgetForEvent(const Event &event) override;
    virtual WidgetPtr GetCurrentWidget() const override {
        return hovered_widget_;
    }
    virtual void SetActive(bool is_active) override;
    virtual bool MovedEnoughForDrag(const Event &event) override;
    virtual void FillActivationDragInfo(DragInfo &info) override;
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) override;
    virtual void FillClickInfo(ClickInfo &info) override;
    virtual void Reset() override;

  private:
    Ray           current_ray_;     ///< Ray for the current Event.
    SG::Hit       current_hit_;     ///< Hit for the current Event.
    Ray           activation_ray_;  ///< Ray for the Event at activation.
    SG::Hit       activation_hit_;  ///< Hit for the Event at activation.
    WidgetPtr     hovered_widget_;  ///< From last call to GetWidgetForEvent().

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

WidgetPtr MHPointerHelper_::GetWidgetForEvent(const Event &event) {
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

void MHPointerHelper_::SetActive(bool is_active) {
    if (is_active) {
        activation_ray_ = current_ray_;
        activation_hit_ = current_hit_;
    }
    if (controller_) {
        const auto &context = GetContext();
        controller_->ShowActive(is_active, false);
        const auto &other_controller = controller_ == context.left_controller ?
            context.right_controller : context.left_controller;
        other_controller->ShowPointer(! is_active);
        other_controller->ShowGrip(! is_active);
    }
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

void MHPointerHelper_::FillClickInfo(ClickInfo &info) {
    const auto actuator = GetActuator();
    info.device = actuator == Actuator_::kMouse ? Event::Device::kMouse :
        actuator == Actuator_::kLeftPinch ? Event::Device::kLeftController :
        Event::Device::kRightController;
    info.hit = current_hit_;
    info.widget = Util::CastToDerived<ClickableWidget>(hovered_widget_).get();
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

    virtual WidgetPtr GetWidgetForEvent(const Event &event) override;
    virtual WidgetPtr GetCurrentWidget() const override {
        return hovered_widget_;
    }
    virtual void SetActive(bool is_active) override;
    virtual bool MovedEnoughForDrag(const Event &event) override;
    virtual void FillActivationDragInfo(DragInfo &info) override;
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) override;
    virtual void FillClickInfo(ClickInfo &info) override;
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
    WidgetPtr     hovered_widget_;   ///< From last call to GetWidgetForEvent().

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

WidgetPtr MHGripHelper_::GetWidgetForEvent(const Event &event) {
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

void MHGripHelper_::SetActive(bool is_active) {
    if (is_active)
        activation_data_ = current_data_;

    const auto &context = GetContext();
    controller_->ShowActive(is_active, true);
    const auto &other_controller = controller_ == context.left_controller ?
        context.right_controller : context.left_controller;
    other_controller->ShowPointer(! is_active);
    other_controller->ShowGrip(! is_active);

    if (grippable_)
        grippable_->ActivateGrip(controller_->GetHand(), is_active);
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

void MHGripHelper_::FillClickInfo(ClickInfo &info) {
    info.device = GetActuator() == Actuator_::kLeftGrip ?
        Event::Device::kLeftController : Event::Device::kRightController;
    info.widget = Util::CastToDerived<ClickableWidget>(hovered_widget_).get();
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

    virtual WidgetPtr GetWidgetForEvent(const Event &event) override;
    virtual WidgetPtr GetCurrentWidget() const override {
        return touched_widget_;
    }
    virtual void SetActive(bool is_active) override;
    virtual bool MovedEnoughForDrag(const Event &event) override;
    virtual void FillActivationDragInfo(DragInfo &info) override;
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) override;
    virtual void FillClickInfo(ClickInfo &info) override;
    virtual void Reset() override;

  private:
    WidgetPtr     touched_widget_;   ///< From last call to GetWidgetForEvent().

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

WidgetPtr MHTouchHelper_::GetWidgetForEvent(const Event &event) {
    WidgetPtr widget;
    // XXXX
    return widget;
}

void MHTouchHelper_::SetActive(bool is_active) {
    // XXXX
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

void MHTouchHelper_::FillClickInfo(ClickInfo &info) {
    info.device = GetActuator() == Actuator_::kLeftTouch ?
        Event::Device::kLeftController : Event::Device::kRightController;
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
    std::vector<std::shared_ptr<MHHelper_>> helpers_;

    // ------------------------------------------------------------------------
    // Functions.

    /// Used by the constructor to set up all of the helpers.
    void InitHelpers_();

    /// Returns the helper associated with an Actuator_.
    MHHelper_ & GetHelper_(Actuator_ actuator) const;

    /// Returns true if the event is a button press or touch on an enabled
    /// Widget.
    bool IsActivationEvent_(const Event &event);

    /// Returns true if the event is a deactivation of the current actuator.
    bool IsDeactivationEvent_(const Event &event);

    /// Processes an event that causes activation of an actuator.
    void ProcessActivationEvent_(const Event &event);

    /// Processes an event that causes deactivation of the active actuator.
    void ProcessDeactivationEvent_(const Event &event);

    /// This is called when the handler is activated or dragging. It checks the
    /// given event for both the start of a new drag or continuation of a
    /// current drag. It returns true if either was true.
    bool StartOrContinueDrag_(const Event &event);

    /// Starts or continues a drag operation.
    void ProcessDrag_(const Event &event, bool is_start,
                      bool is_alternate_mode);

    /// Processes a click using the given actuator.
    void ProcessClick_(Actuator_ actuator, bool is_alternate_mode);

    /// Resets everything after it is known that a click has finished: the
    /// timer is no longer running.
    void ResetClick_();

    /// Returns the Actuator_ corresponding to the given event, which is known
    /// to cause activation.
    static Actuator_ GetActuatorForEvent_(const Event &event);

    /// Returns true if the given Widget (which may be null) is draggable.
    static bool IsDraggableWidget_(const WidgetPtr &widget) {
        return Util::CastToDerived<DraggableWidget>(widget).get();
    }

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
    for (auto &helper: helpers_)
        helper->SetPathFilter(filter);
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

        // Set up grip helpers.
        const SG::NodePath path = cur_grippable_ ?
            SG::FindNodePathInScene(*context_->scene, *cur_grippable_) :
            SG::NodePath();
        auto get_helper = [&](Actuator_ act){
            const auto &helper = helpers_[Util::EnumInt(act)];
            return *Util::CastToDerived<MHGripHelper_>(helper);
        };
        get_helper(Actuator_::kLeftGrip).SetGrippable(cur_grippable_, path);
        get_helper(Actuator_::kRightGrip).SetGrippable(cur_grippable_, path);
    }

    // If the click timer finishes and not in the middle of another click or
    // drag, process the click. If not, then clear _activeData.
    if (click_state_.timer.IsFinished()) {
        if (IsWaiting()) {
            if (click_state_.count > 0)
                ProcessClick_(click_state_.actuator, is_alternate_mode);
            ResetClick_();
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
            for (auto &helper: helpers_)
                helper->GetWidgetForEvent(event);
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
    for (auto &helper: helpers_)
        helper->Reset();

    click_state_.Reset();

    state_                 = State_::kWaiting;
    cur_actuator_          = Actuator_::kNone;
    moved_enough_for_drag_ = false;
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

bool MainHandler::Impl_::IsDeactivationEvent_(const Event &event) {
    return event.flags.Has(Event::Flag::kButtonRelease) &&
        GetActuatorForEvent_(event) == cur_actuator_;
}

void MainHandler::Impl_::ProcessActivationEvent_(const Event &event) {
    // Determine the Actuator_ for the event.
    ASSERT(cur_actuator_ == Actuator_::kNone);
    cur_actuator_ = GetActuatorForEvent_(event);
    KLOG('h', "MainHandler kActivated by " << Util::EnumName(event.device));

    // Stop hovering with all actuators.
    for (auto &helper: helpers_) {
        if (auto widget = helper->GetCurrentWidget())
            widget->SetHovering(false);
    }

    // Get the active helper, mark it as active, and and ask it for the current
    // Widget.
    auto &helper = GetHelper_(cur_actuator_);
    helper.SetActive(true);
    WidgetPtr widget = helper.GetCurrentWidget();

    // If the click timer is currently running and this is the same button,
    // this is a multiple click.
    if (click_state_.IsMultipleClick(cur_actuator_))
        ++click_state_.count;
    else
        click_state_.count = 1;

    // Set a timeout only if the click is on a Widget that is draggable.
    // Otherwise, just process the click immediately.
    const float timeout = IsDraggableWidget_(widget) ? kClickTimeout_ : 0;

    start_time_ = UTime::Now();
    click_state_.actuator = cur_actuator_;
    click_state_.timer.Start(timeout);

    moved_enough_for_drag_ = false;

    if (widget)
        widget->SetActive(true);
}

void MainHandler::Impl_::ProcessDeactivationEvent_(const Event &event) {
    ASSERT(cur_actuator_ != Actuator_::kNone);
    auto &helper = GetHelper_(cur_actuator_);
    helper.SetActive(false);

    WidgetPtr widget = helper.GetCurrentWidget();
    ASSERT(widget);
    widget->SetActive(false);

    if (state_ == State_::kDragging) {
        auto draggable = Util::CastToDerived<DraggableWidget>(widget);
        ASSERT(draggable);
        draggable->EndDrag();
    }

    // If the click timer is not already running, process the event.
    if (! click_state_.timer.IsRunning()) {
        // The deactivation represents a click if all of the following are
        // true:
        //   - A drag is not in process.
        //   - The active device did not move too much (enough for a drag).
        //   - The active device did not move off the clickable widget.
        const bool is_click = state_ != State_::kDragging &&
            ! moved_enough_for_drag_ &&
            helper.GetWidgetForEvent(event) == widget;

        // If the timer is not running, process the click if it is one, and
        // always reset everything.
        if (is_click)
            ProcessClick_(cur_actuator_, event.is_alternate_mode);
        ResetClick_();
    }
    state_ = State_::kWaiting;
    cur_actuator_ = Actuator_::kNone;
    KLOG('h', "MainHandler kWaiting after deactivation");
}

bool MainHandler::Impl_::StartOrContinueDrag_(const Event &event) {
    ASSERT(cur_actuator_ != Actuator_::kNone);
    ASSERT(state_ == State_::kActivated || state_ == State_::kDragging);

    // See if this is the start of a new drag.
    auto &helper = GetHelper_(cur_actuator_);
    const bool is_drag_start =
        state_ == State_::kActivated && helper.MovedEnoughForDrag(event);

    if (is_drag_start || state_ == State_::kDragging) {
        state_ = State_::kDragging;
        ProcessDrag_(event, is_drag_start, event.is_alternate_mode);
        return true;
    }
    return false;
}

void MainHandler::Impl_::ProcessDrag_(const Event &event, bool is_start,
                                      bool is_alternate_mode) {
    ASSERT(state_ == State_::kDragging);
    ASSERT(moved_enough_for_drag_);
    ASSERT(cur_actuator_ != Actuator_::kNone);

    // Set common items in DragInfo.
    drag_info_.is_alternate_mode = is_alternate_mode || click_state_.count > 1;
    drag_info_.linear_precision  = precision_manager_->GetLinearPrecision();
    drag_info_.angular_precision = precision_manager_->GetAngularPrecision();

    // Let the helper set up the rest.
    auto &helper = GetHelper_(cur_actuator_);

    auto draggable =
        Util::CastToDerived<DraggableWidget>(helper.GetCurrentWidget());
    ASSERT(draggable);

    // If starting a new drag.
    if (is_start) {
        // These are set once and used throughout the drag.
        drag_info_.path_to_stage  = context_->path_to_stage;
        drag_info_.path_to_widget =
            SG::FindNodePathInScene(*context_->scene, *draggable);

        helper.FillActivationDragInfo(drag_info_);

        draggable->SetHovering(false);
        draggable->StartDrag(drag_info_);

        KLOG('h', "MainHandler kDragging with " << draggable->GetDesc()
             << " (" << drag_info_.path_to_widget.ToString() << ")");
    }

    // Continuing a current drag operation.
    else {
        helper.FillEventDragInfo(event, drag_info_);
        draggable->ContinueDrag(drag_info_);
    }
}

void MainHandler::Impl_::ProcessClick_(Actuator_ actuator,
                                       bool is_alternate_mode) {
    ASSERT(actuator != Actuator_::kNone);
    auto &helper = GetHelper_(actuator);

    ClickInfo info;
    info.is_alternate_mode = is_alternate_mode || click_state_.count > 1;
    info.is_long_press     =
        UTime::Now().SecondsSince(start_time_) > kLongPressTime_;

    helper.FillClickInfo(info);

    clicked_.Notify(info);

    state_ = State_::kWaiting;
    KLOG('h', "MainHandler kWaiting after click on "
         << info.hit.path.ToString());
}

void MainHandler::Impl_::ResetClick_() {
    ASSERT(! click_state_.timer.IsRunning());

    // Indicate that the device is no longer active.
    ASSERT(click_state_.actuator != Actuator_::kNone);
    auto &helper = GetHelper_(click_state_.actuator);
    helper.SetActive(false);
    cur_actuator_ = Actuator_::kNone;
    click_state_.Reset();
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
