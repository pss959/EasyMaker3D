#include "Handlers/MainHandler.h"

#include <vector>

#include "App/DragInfo.h"
#include "App/SceneContext.h"
#include "Base/Event.h"
#include "Debug/Print.h"
#include "Enums/Actuator.h"
#include "Items/Controller.h"
#include "Items/Grippable.h"
#include "Managers/PrecisionManager.h"
#include "Math/Types.h"
#include "SG/Search.h"
#include "Trackers/GripTracker.h"
#include "Trackers/MouseTracker.h"
#include "Trackers/PinchTracker.h"
#include "Trackers/TouchTracker.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/Timer.h"
#include "Util/UTime.h"
#include "Widgets/ClickableWidget.h"
#include "Widgets/DraggableWidget.h"

namespace {

// ----------------------------------------------------------------------------
// ClickState_ struct.
// ----------------------------------------------------------------------------

/// ClickState_ saves information about a current potential click in progress.
struct ClickState_ {
    Timer     timer;      ///< Used to detect multiple clicks.
    int       count = 0;  ///< Current number of clicks.
    Actuator actuator;   ///< Actuator that started the current click.

    /// Copy of the Event that causes deactivation (once it is known).
    Event     deactivation_event;

    ClickState_() { Reset(); }

    void Reset() {
        count    = 0;
        actuator = Actuator::kNone;
    }

    /// Returns true if the timer is currently running and the passed Actuator
    /// matches what is stored here, meaning this is a multiple click.
    bool IsMultipleClick(Actuator actuator_from_event) const {
        return timer.IsRunning() && actuator_from_event == actuator;
    }
};

}  // anonymous namespace

// ----------------------------------------------------------------------------
// MainHandler::Impl_ class.
// ----------------------------------------------------------------------------

class MainHandler::Impl_ {
  public:
    Impl_() { InitTrackers_(); }
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

    /// Actuator that caused the current activation, if any.
    Actuator   cur_actuator_ = Actuator::kNone;

    /// This is set to true after activation if the device moved enough to be
    /// considered a drag operation.
    bool        moved_enough_for_drag_ = false;

    /// DragInfo instance used to process drags.
    DragInfo    drag_info_;

    /// Stores events produced by touches to simulate pinches. XXXX Needed?
    std::vector<Event> extra_events_;

    /// Actuator trackers. There is one stored for each Actuator value except
    /// Actuator::kNone.
    std::vector<TrackerPtr> trackers_;

    // ------------------------------------------------------------------------
    // Functions.

    /// Used by the constructor to set up all of the trackers.
    void InitTrackers_();

    /// Returns the tracker associated with an Actuator.
    Tracker & GetTracker(Actuator actuator) const;

    /// Returns a tracker of a given type.
    template <typename T> T & GetTypedTracker(Actuator actuator) const;

    /// Returns true if the event is a button press or touch on an enabled
    /// Widget.
    bool IsActivationEvent_(const Event &event);

    /// Returns true if the event is a deactivation of the current actuator.
    bool IsDeactivationEvent_(const Event &event);

    /// Processes activation of an actuator.
    void ProcessActivation_(Actuator actuator);

    /// Processes deactivation of the active actuator.
    void ProcessDeactivation_(bool is_alternate_mode);

    /// This is called when the handler is activated or dragging. It checks the
    /// given event for both the start of a new drag or continuation of a
    /// current drag. It returns true if either was true.
    bool StartOrContinueDrag_(const Event &event);

    /// Starts or continues a drag operation.
    void ProcessDrag_(const Event &event, bool is_start,
                      bool is_alternate_mode);

    /// Processes a click using the given actuator.
    void ProcessClick_(Actuator actuator, bool is_alternate_mode);

    /// Resets everything after it is known that a click has finished: the
    /// timer is no longer running.
    void ResetClick_();

    /// Returns the Actuator corresponding to the given event, which is known
    /// to cause activation.
    static Actuator GetActuatorForEvent_(const Event &event);

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
    for (auto &tracker: trackers_)
        tracker->SetSceneContext(context);
}

void MainHandler::Impl_::SetPathFilter(const PathFilter &filter) {
    auto set_filter = [&](Actuator act){
        GetTypedTracker<PointerTracker>(act).SetPathFilter(filter);
    };
    set_filter(Actuator::kMouse);
    set_filter(Actuator::kLeftPinch);
    set_filter(Actuator::kRightPinch);
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

        // Set up grip trackers.
        const SG::NodePath path = cur_grippable_ ?
            SG::FindNodePathInScene(*context_->scene, *cur_grippable_) :
            SG::NodePath();
        auto set_grippable = [&](Actuator act){
            GetTypedTracker<GripTracker>(act).SetGrippable(cur_grippable_,
                                                           path);
        };
        set_grippable(Actuator::kLeftGrip);
        set_grippable(Actuator::kRightGrip);
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
        ASSERT(cur_actuator_ == Actuator::kNone);
        if (IsActivationEvent_(event)) {
            ProcessActivation_(GetActuatorForEvent_(event));
            handled = true;
        }
        // If waiting for a potential end of a click, do nothing (so as not to
        // mess up the active state).
        else if (! click_state_.timer.IsRunning()) {
            // Check for touches first: if a Widget is touched, this also
            // counts as an activation.
            if (GetTracker(Actuator::kLeftTouch).GetWidgetForEvent(event)) {
                ProcessActivation_(Actuator::kLeftTouch);
            }
            else if (GetTracker(
                         Actuator::kRightTouch).GetWidgetForEvent(event)) {
                ProcessActivation_(Actuator::kRightTouch);
            }

            // Otherwise, update the hover state for all other actuators.
            else {
                GetTracker(Actuator::kMouse).GetWidgetForEvent(event);
                GetTracker(Actuator::kLeftPinch).GetWidgetForEvent(event);
                GetTracker(Actuator::kRightPinch).GetWidgetForEvent(event);
                GetTracker(Actuator::kLeftGrip).GetWidgetForEvent(event);
                GetTracker(Actuator::kRightGrip).GetWidgetForEvent(event);
            }
        }
    }

    // Either State_::kActivated or State_::kDragging.
    else {
        if (IsDeactivationEvent_(event)) {
            ProcessDeactivation_(event.is_alternate_mode);
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
    for (auto &tracker: trackers_)
        tracker->Reset();

    click_state_.Reset();

    state_                 = State_::kWaiting;
    cur_actuator_          = Actuator::kNone;
    moved_enough_for_drag_ = false;
}

void MainHandler::Impl_::InitTrackers_() {
    // Shorthand.
    auto get_index = [](Actuator act){ return Util::EnumInt(act); };

    // Make room for all trackers.
    const int tracker_count = Util::EnumCount<Actuator>() - 1;
    ASSERT(tracker_count == get_index(Actuator::kNone));
    trackers_.resize(tracker_count);

    // Shorthand
#define SET_TRACKER_(act, type)                                          \
    trackers_[get_index(Actuator::act)].reset(new type(Actuator::act))

    SET_TRACKER_(kMouse,      MouseTracker);
    SET_TRACKER_(kLeftPinch,  PinchTracker);
    SET_TRACKER_(kRightPinch, PinchTracker);
    SET_TRACKER_(kLeftGrip,   GripTracker);
    SET_TRACKER_(kRightGrip,  GripTracker);
    SET_TRACKER_(kLeftTouch,  TouchTracker);
    SET_TRACKER_(kRightTouch, TouchTracker);

#undef SET_TRACKER_
}

Tracker & MainHandler::Impl_::GetTracker(Actuator actuator) const {
    ASSERT(actuator != Actuator::kNone);
    return *trackers_[Util::EnumInt(actuator)];
}

template <typename T> T &
MainHandler::Impl_::GetTypedTracker(Actuator actuator) const {
    ASSERT(actuator != Actuator::kNone);
    std::shared_ptr<T> tracker =
        Util::CastToDerived<T>(trackers_[Util::EnumInt(actuator)]);
    ASSERT(tracker);
    return *tracker;
}

bool MainHandler::Impl_::IsActivationEvent_(const Event &event) {
    return event.flags.Has(Event::Flag::kButtonPress) &&
        GetActuatorForEvent_(event) != Actuator::kNone;
}

bool MainHandler::Impl_::IsDeactivationEvent_(const Event &event) {
    // Touches are handled specially - if the touch is no longer on the
    // activated Widget, deactivate it.
    ASSERT(cur_actuator_ != Actuator::kNone);
    if (cur_actuator_ == Actuator::kLeftTouch ||
        cur_actuator_ == Actuator::kRightTouch) {
        auto &tracker = GetTracker(cur_actuator_);
        WidgetPtr cur_widget = tracker.GetCurrentWidget();
        return tracker.GetWidgetForEvent(event) != cur_widget;
    }

    // Everything else requires the correct button release.
    return event.flags.Has(Event::Flag::kButtonRelease) &&
        GetActuatorForEvent_(event) == cur_actuator_;
}

void MainHandler::Impl_::ProcessActivation_(Actuator actuator) {
    // Determine the Actuator for the event.
    ASSERT(cur_actuator_ == Actuator::kNone);
    cur_actuator_ = actuator;

    // Stop hovering with all actuators.
    for (auto &tracker: trackers_) {
        if (auto widget = tracker->GetCurrentWidget()) {
            if (widget->IsHovering())
                widget->SetHovering(false);
        }
    }

    // Get the active tracker, mark it as active, and and ask it for the current
    // Widget.
    auto &tracker = GetTracker(cur_actuator_);
    tracker.SetActive(true);
    WidgetPtr widget = tracker.GetCurrentWidget();
    KLOG('h', "MainHandler kActivated by " << Util::EnumName(cur_actuator_)
         << " on " << (widget ? widget->GetDesc() : "<NO WIDGET>"));

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

    state_ = State_::kActivated;
}

void MainHandler::Impl_::ProcessDeactivation_(bool is_alternate_mode) {
    ASSERT(cur_actuator_ != Actuator::kNone);
    auto &tracker = GetTracker(cur_actuator_);
    tracker.SetActive(false);

    WidgetPtr widget = tracker.GetCurrentWidget();
    if (widget)
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
        //   - A drag is not in progress.
        //   - The active device did not move too much (enough for a drag).
        //   - The active device did not move off the clickable widget.
        const bool is_click = state_ != State_::kDragging &&
            ! moved_enough_for_drag_ &&
            tracker.GetCurrentWidget() == widget;

        // If the timer is not running, process the click if it is one, and
        // always reset everything.
        if (is_click)
            ProcessClick_(cur_actuator_, is_alternate_mode);
        ResetClick_();
    }
    state_ = State_::kWaiting;
    cur_actuator_ = Actuator::kNone;
    KLOG('h', "MainHandler kWaiting after deactivation");
}

bool MainHandler::Impl_::StartOrContinueDrag_(const Event &event) {
    ASSERT(cur_actuator_ != Actuator::kNone);
    ASSERT(state_ == State_::kActivated || state_ == State_::kDragging);

    // If a drag has not started, check for enough motion for a drag. Do this
    // even if the Widget is not draggable, since sufficient motion should
    // cancel a click on a ClickableWidget as well.
    auto &tracker = GetTracker(cur_actuator_);
    if (state_ == State_::kActivated && ! moved_enough_for_drag_)
        moved_enough_for_drag_ = tracker.MovedEnoughForDrag(event);

    // If the Widget is not draggable, stop.
    if (! IsDraggableWidget_(tracker.GetCurrentWidget()))
        return false;

    // See if this is the start of a new drag.
    const bool is_drag_start =
        state_ == State_::kActivated && moved_enough_for_drag_;

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
    ASSERT(cur_actuator_ != Actuator::kNone);

    // Set common items in DragInfo.
    drag_info_.is_alternate_mode = is_alternate_mode || click_state_.count > 1;
    drag_info_.linear_precision  = precision_manager_->GetLinearPrecision();
    drag_info_.angular_precision = precision_manager_->GetAngularPrecision();

    // Let the tracker set up the rest.
    auto &tracker = GetTracker(cur_actuator_);

    auto draggable =
        Util::CastToDerived<DraggableWidget>(tracker.GetCurrentWidget());
    ASSERT(draggable);
    ASSERT(! draggable->IsHovering());

    // If starting a new drag.
    if (is_start) {
        // These are set once and used throughout the drag.
        drag_info_.path_to_stage  = context_->path_to_stage;
        drag_info_.path_to_widget =
            SG::FindNodePathInScene(*context_->scene, *draggable);

        tracker.FillActivationDragInfo(drag_info_);

        draggable->StartDrag(drag_info_);
        KLOG('h', "MainHandler kDragging with " << draggable->GetDesc()
             << " (" << drag_info_.path_to_widget.ToString() << ")");
    }

    // Continuing a current drag operation.
    else {
        tracker.FillEventDragInfo(event, drag_info_);
        draggable->ContinueDrag(drag_info_);
    }
}

void MainHandler::Impl_::ProcessClick_(Actuator actuator,
                                       bool is_alternate_mode) {
    ASSERT(actuator != Actuator::kNone);
    auto &tracker = GetTracker(actuator);

    ClickInfo info;
    info.is_alternate_mode = is_alternate_mode || click_state_.count > 1;
    info.is_long_press     =
        UTime::Now().SecondsSince(start_time_) > kLongPressTime_;

    tracker.FillClickInfo(info);

    clicked_.Notify(info);

    state_ = State_::kWaiting;
    KLOG('h', "MainHandler kWaiting after click on "
         << info.hit.path.ToString());
}

void MainHandler::Impl_::ResetClick_() {
    ASSERT(! click_state_.timer.IsRunning());

    // Indicate that the device is no longer active.
    ASSERT(click_state_.actuator != Actuator::kNone);
    auto &tracker = GetTracker(click_state_.actuator);
    tracker.SetActive(false);
    cur_actuator_ = Actuator::kNone;
    click_state_.Reset();
}

Actuator MainHandler::Impl_::GetActuatorForEvent_(const Event &event) {
    Actuator act = Actuator::kNone;
    if (event.device == Event::Device::kMouse) {
        act = Actuator::kMouse;
    }
    else if (event.device == Event::Device::kLeftController) {
        if (event.button == Event::Button::kPinch)
            act = Actuator::kLeftPinch;
        else if (event.button == Event::Button::kGrip)
            act = Actuator::kLeftGrip;
    }
    else if (event.device == Event::Device::kRightController) {
        if (event.button == Event::Button::kPinch)
            act = Actuator::kRightPinch;
        else if (event.button == Event::Button::kGrip)
            act = Actuator::kRightGrip;
    }
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
