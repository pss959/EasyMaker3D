#include "Handlers/MainHandler.h"

#include <vector>

#include "Base/Event.h"
#include "Enums/Actuator.h"
#include "Items/Controller.h"
#include "Items/Grippable.h"
#include "Math/Types.h"
#include "Place/DragInfo.h"
#include "Place/PrecisionStore.h"
#include "SG/Search.h"
#include "Trackers/GripTracker.h"
#include "Trackers/MouseTracker.h"
#include "Trackers/PinchTracker.h"
#include "Trackers/TouchTracker.h"
#include "Util/Alarm.h"
#include "Util/Assert.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"
#include "Util/UTime.h"
#include "Widgets/ClickableWidget.h"
#include "Widgets/DraggableWidget.h"

namespace {

// ----------------------------------------------------------------------------
// ClickState_ struct.
// ----------------------------------------------------------------------------

/// ClickState_ saves information about a current potential click in progress.
struct ClickState_ {
    Alarm     alarm;      ///< Used to detect multiple clicks.
    int       count = 0;  ///< Current number of clicks.
    Actuator  actuator;   ///< Actuator that started the current click.

    /// Copy of the Event that causes deactivation (once it is known).
    Event     deactivation_event;

    ClickState_() { Reset(); }

    void Reset() {
        alarm.Stop();
        count    = 0;
        actuator = Actuator::kNone;
    }

    /// Returns true if the alarm is currently running and the passed Actuator
    /// matches what is stored here, meaning this is a multiple click.
    bool IsMultipleClick(Actuator actuator_from_event) const {
        return alarm.IsRunning() && actuator_from_event == actuator;
    }
};

}  // anonymous namespace

// ----------------------------------------------------------------------------
// MainHandler::Impl_ class.
// ----------------------------------------------------------------------------

class MainHandler::Impl_ {
  public:
    Impl_(bool is_vr_enabled) {
        if (is_vr_enabled)
            InitVRTrackers_();
        else
            InitNonVRTrackers_();
    }
    void SetPrecisionStore(const PrecisionStorePtr &precision_store) {
        precision_store_ = precision_store;
    }
    void SetContext(const MainHandler::Context &context);
    void AddGrippable(const GrippablePtr &grippable) {
        grippables_.push_back(grippable);
    }
    void SetTouchable(const ITouchablePtr &touchable);
    Util::Notifier<const ClickInfo &> & GetClicked() {
        return clicked_;
    }
    Util::Notifier<Event::Device, float> & GetValuatorChanged() {
        return valuator_changed_;
    }
    Event::Device GetActiveDevice() const {
        return cur_tracker_ ? cur_tracker_->GetDevice() :
            Event::Device::kUnknown;
    }
    Event::Device GetLastActiveDevice() const {
        return last_tracker_ ? last_tracker_->GetDevice() :
            Event::Device::kUnknown;
    }
    bool IsWaiting() const {
        return state_ == State_::kWaiting && ! click_state_.alarm.IsRunning();
    }
    void SetPathFilter(const PathFilter &filter);
    void ProcessUpdate(bool is_modified_mode);
    bool HandleEvent(const Event &event);
    void Reset();

  private:
    /// Possible states.
    enum class State_ {
        kWaiting,    ///< Waiting for activation events.
        kActivated,  ///< Activation button pressed, but not dragging.
        kDragging,   ///< Activated and sufficient motion for dragging.
    };

    /// Current state.
    State_ state_ = State_::kWaiting;

    /// PrecisionStore used for accessing precision details.
    PrecisionStorePtr precision_store_;

    /// MainHandler::Context storing required data.
    MainHandler::Context context_;

    /// Ordered set of Grippable instances for interaction.
    std::vector<GrippablePtr> grippables_;

    /// Current Grippable: the first one that is enabled.
    GrippablePtr cur_grippable_;

    /// Node returned by GetGrippableNode() for the current Grippable.
    const SG::Node *cur_grippable_node_ = nullptr;

    /// Notifies when a click is detected.
    Util::Notifier<const ClickInfo &> clicked_;

    /// Notifies when a valuator change is detected.
    Util::Notifier<Event::Device, float> valuator_changed_;

    /// Time at which the current device was activated.
    UTime       start_time_;

    /// Information used to detect and process clicks.
    ClickState_ click_state_;

    /// Tracker that manages the current activated actuator, or null if there
    /// is none in progress.
    TrackerPtr  cur_tracker_;

    /// Tracker that managed the last activated actuator, or null if there was
    /// none.
    TrackerPtr  last_tracker_;

    /// If a click or drag is in progress, this stores the Widget that is
    /// involved.
    WidgetPtr   active_widget_;

    /// This is set to true after activation if the device moved enough to be
    /// considered a drag operation.
    bool        moved_enough_for_drag_ = false;

    /// DragInfo instance used to process drags.
    DragInfo    drag_info_;

    /// Actuator trackers. There is one stored for each Actuator value except
    /// Actuator::kNone.
    std::vector<TrackerPtr> trackers_;

    // ------------------------------------------------------------------------
    // Functions.

    /// Used by the constructor to set up the required trackers for VR.
    void InitVRTrackers_();
    /// Used by the constructor to set up the required trackers for non-VR.
    void InitNonVRTrackers_();

    /// Returns true if VR Trackers are enabled.
    bool IsVREnabled_() const { return trackers_.size() > 1U; }

    /// Returns the tracker associated with an Actuator.
    const TrackerPtr & GetTracker_(Actuator actuator) const;

    /// Returns a tracker of a given type.
    template <typename T> T & GetTypedTracker_(Actuator actuator) const;

    /// Returns the active Grippable, if any.
    GrippablePtr GetActiveGrippable_();

    /// Updates the current Grippable and grip guide if necessary.
    void UpdateGrippable_();

    /// Updates hovering of all Trackers based on the given Event.
    void UpdateHovering_(const Event &event);

    /// If the Event represents an activation of any Tracker, this processes
    /// the activation and returns true.
    bool Activate_(const Event event);

    /// If the Event represents a deactivation of the current Tracker, this
    /// processes the deactivation and returns true.
    bool Deactivate_(const Event &event);

    /// Processes activation using the current Tracker.
    void ProcessActivation_();

    /// Processes deactivation using the current Tracker. The is_on_same_widget
    /// indicates whether the deactivation Event is over the same Widget that
    /// was used for activation. (If they differ, it cannot be a click.)
    void ProcessDeactivation_(bool is_modified_mode, bool is_on_same_widget);

    /// This is called when the handler is activated or dragging. It checks the
    /// given event for both the start of a new drag or continuation of a
    /// current drag. It returns true if either was true.
    bool StartOrContinueDrag_(const Event &event);

    /// Starts or continues a drag operation.
    void ProcessDrag_(const Event &event, bool is_start,
                      bool is_modified_mode);

    /// Processes a click using the given actuator.
    void ProcessClick_(Actuator actuator, bool is_modified_mode);

    /// Resets everything after it is known that a click has finished: the
    /// alarm is no longer running.
    void ResetClick_();

    /// Returns true if the given Widget (which may be null) is draggable.
    static bool IsDraggableWidget_(const WidgetPtr &widget) {
        return std::dynamic_pointer_cast<DraggableWidget>(widget).get();
    }
};

// ----------------------------------------------------------------------------
// MainHandler::Impl_ implementation.
// ----------------------------------------------------------------------------

void MainHandler::Impl_::SetContext(const MainHandler::Context &context) {
    // Copy the Context data.
    context_ = context;

    for (auto &tracker: trackers_)
        tracker->Init(context_.scene,
                      context_.left_controller, context_.right_controller);

    // Special initialization for the MouseTracker.
    auto mt =
        std::dynamic_pointer_cast<MouseTracker>(GetTracker_(Actuator::kMouse));
    mt->SetFrustum(context_.frustum);
    mt->SetDebugSphere(context_.debug_sphere);
}

void MainHandler::Impl_::SetTouchable(const ITouchablePtr &touchable) {
    if (IsVREnabled_()) {
        auto set_touchable = [&](Actuator act){
            GetTypedTracker_<TouchTracker>(act).SetTouchable(touchable);
        };
        set_touchable(Actuator::kLeftTouch);
        set_touchable(Actuator::kRightTouch);
    }
}

void MainHandler::Impl_::SetPathFilter(const PathFilter &filter) {
    auto set_filter = [&](Actuator act){
        GetTypedTracker_<PointerTracker>(act).SetPathFilter(filter);
    };
    set_filter(Actuator::kMouse);
    if (IsVREnabled_()) {
        set_filter(Actuator::kLeftPinch);
        set_filter(Actuator::kRightPinch);
    }
}

void MainHandler::Impl_::ProcessUpdate(bool is_modified_mode) {
    // Always call UpdateGrippable_() in case the grip guide changed.
    UpdateGrippable_();

    // If the click alarm finishes and not in the middle of another click or
    // drag, process the click. If not, then reset the click.
    if (click_state_.alarm.IsFinished()) {
        if (IsWaiting()) {
            if (click_state_.count > 0)
                ProcessClick_(click_state_.actuator, is_modified_mode);
            ResetClick_();
        }
    }
}

bool MainHandler::Impl_::HandleEvent(const Event &event) {
    ASSERT(context_.scene);

    bool handled = false;

    // Valuator events are handled specially.
    if (event.flags.Has(Event::Flag::kPosition1D)) {
        valuator_changed_.Notify(event.device, event.position1D);
        handled = true;
    }

    // If not in the middle of a click or drag.
    else if (state_ == State_::kWaiting) {
        if (Activate_(event))
            handled = true;
        else
            UpdateHovering_(event);
    }

    // Either State_::kActivated or State_::kDragging.
    else {
        handled = Deactivate_(event) || StartOrContinueDrag_(event);
    }

    return handled;
}

void MainHandler::Impl_::Reset() {
    for (auto &tracker: trackers_)
        tracker->Reset();

    state_                 = State_::kWaiting;
    moved_enough_for_drag_ = false;

    click_state_.Reset();
    cur_tracker_.reset();
    last_tracker_.reset();
}

void MainHandler::Impl_::InitVRTrackers_() {
    // Shorthand.
    auto get_index = [](Actuator act){ return Util::EnumInt(act); };

#define SET_TRACKER_(act, type)                                         \
    trackers_[get_index(Actuator::act)].reset(new type(Actuator::act))

    // Make room for all trackers.
    const int tracker_count = Util::EnumCount<Actuator>() - 1;
    ASSERT(tracker_count == get_index(Actuator::kNone));
    trackers_.resize(tracker_count);
    SET_TRACKER_(kMouse,      MouseTracker);
    SET_TRACKER_(kLeftPinch,  PinchTracker);
    SET_TRACKER_(kRightPinch, PinchTracker);
    SET_TRACKER_(kLeftGrip,   GripTracker);
    SET_TRACKER_(kRightGrip,  GripTracker);
    SET_TRACKER_(kLeftTouch,  TouchTracker);
    SET_TRACKER_(kRightTouch, TouchTracker);

#undef SET_TRACKER_
}

void MainHandler::Impl_::InitNonVRTrackers_() {
    // Just add the MouseTracker.
    ASSERT(Util::EnumInt(Actuator::kMouse) == 0);
    trackers_.push_back(TrackerPtr(new MouseTracker(Actuator::kMouse)));
}

const TrackerPtr & MainHandler::Impl_::GetTracker_(Actuator actuator) const {
    ASSERT(actuator != Actuator::kNone);
    return trackers_[Util::EnumInt(actuator)];
}

template <typename T> T &
MainHandler::Impl_::GetTypedTracker_(Actuator actuator) const {
    ASSERT(actuator != Actuator::kNone);
    std::shared_ptr<T> tracker =
        std::dynamic_pointer_cast<T>(trackers_[Util::EnumInt(actuator)]);
    ASSERT(tracker);
    return *tracker;
}

GrippablePtr MainHandler::Impl_::GetActiveGrippable_() {
    GrippablePtr active_grippable;
    for (auto &grippable: grippables_) {
        if (grippable->GetGrippableNode()) {
            active_grippable = grippable;
            break;
        }
    }
    return active_grippable;
}

void MainHandler::Impl_::UpdateGrippable_() {
    if (! IsVREnabled_())
        return;

    // Determine what the current (enabled) Grippable is, if any, and its node.
    auto grippable = GetActiveGrippable_();
    auto grippable_node = grippable ? grippable->GetGrippableNode() : nullptr;

    // Update trackers if either the grippable or its node changed.
    if (grippable != cur_grippable_ || grippable_node != cur_grippable_node_) {
        SG::NodePath path;
        if (grippable_node)
            path = SG::FindNodePathInScene(*context_.scene, *grippable_node);
        auto set_grippable = [&](Actuator act){
            GetTypedTracker_<GripTracker>(act).SetGrippable(grippable, path);
        };
        set_grippable(Actuator::kLeftGrip);
        set_grippable(Actuator::kRightGrip);
    }
    cur_grippable_      = grippable;
    cur_grippable_node_ = grippable_node;
}

void MainHandler::Impl_::UpdateHovering_(const Event &event) {
    // Update the hover state for all Trackers unless waiting for a potential
    // end of a click, in which case do nothing to avoid messing up the active
    // state.
    if (! click_state_.alarm.IsRunning()) {
        for (auto &tracker: trackers_)
            tracker->UpdateHovering(event);
    }
}

bool MainHandler::Impl_::Activate_(const Event event) {
    ASSERT(! cur_tracker_);
    ASSERT(! active_widget_);

    // Ask each Tracker if the event causes activation.
    WidgetPtr widget;
    for (auto &tracker: trackers_) {
        if (tracker->IsActivation(event, widget)) {
            cur_tracker_   = tracker;
            last_tracker_  = tracker;
            active_widget_ = widget;
            KLOG('h', "MainHandler active widget = "
                 << (widget ? widget->GetDesc() : "NULL")
                 << " from " << Util::EnumName(tracker->GetActuator()));
            if (event.device == Event::Device::kMouse) {
                KLOG('d', "Mouse activation at " << event.position2D
                     << (event.is_modified_mode ? " (MOD)" : ""));
            }
            ProcessActivation_();
            state_ = State_::kActivated;
            KLOG('h', "MainHandler now kActivated");
            moved_enough_for_drag_ = false;
            return true;
        }
    }
    return false;
}

bool MainHandler::Impl_::Deactivate_(const Event &event) {
    ASSERT(cur_tracker_);

    // Ask the current Tracker if the event causes deactivation.
    WidgetPtr widget;
    if (cur_tracker_->IsDeactivation(event, widget)) {
        ProcessDeactivation_(event.is_modified_mode, widget == active_widget_);
        cur_tracker_.reset();
        active_widget_.reset();
        state_ = State_::kWaiting;
        KLOG('d', "MainHandler now kWaiting");
        if (event.device == Event::Device::kMouse) {
            KLOG('d', "Mouse deactivation at " << event.position2D
                     << (event.is_modified_mode ? " (MOD)" : ""));
        }
        return true;
    }
    return false;
}

void MainHandler::Impl_::ProcessActivation_() {
    ASSERT(cur_tracker_);

    // Stop all hovering.
    for (auto &tracker: trackers_)
        tracker->StopHovering();

    const Actuator actuator = cur_tracker_->GetActuator();

    KLOG('h', "MainHandler kActivated by " << Util::EnumName(actuator)
         << " on " << (active_widget_ ? active_widget_->GetDesc() :
                       "<NO WIDGET>"));

    if (active_widget_)
        active_widget_->SetActive(true);

    // If the click alarm is currently running and this is the same button,
    // this is a multiple click.
    if (click_state_.IsMultipleClick(actuator))
        ++click_state_.count;
    else
        click_state_.count = 1;
    KLOG('h', "MainHandler click count = " << click_state_.count);

    // Set a timeout to distinguish from drags and to detect a double click.
    const float timeout = cur_tracker_->GetClickTimeout();

    start_time_ = UTime::Now();
    click_state_.actuator = actuator;
    click_state_.alarm.Start(timeout);
}

void MainHandler::Impl_::ProcessDeactivation_(bool is_modified_mode,
                                              bool is_on_same_widget) {
    ASSERT(cur_tracker_);
    KLOG('h', "MainHandler processing deactivation by "
         << Util::EnumName(cur_tracker_->GetActuator())
         << " state = " << Util::EnumName(state_));

    if (state_ == State_::kDragging) {
        auto draggable =
            std::dynamic_pointer_cast<DraggableWidget>(active_widget_);
        ASSERT(draggable);
        draggable->EndDrag();
        click_state_.Reset();
        KLOG('h', "MainHandler click count = " << click_state_.count);
    }

    if (active_widget_)
        active_widget_->SetActive(false);

    // If the click alarm is not already running, process the event.
    if (! click_state_.alarm.IsRunning()) {
        // The deactivation represents a click if all of the following are
        // true:
        //   - A drag is not in progress.
        //   - The active device did not move too much (enough for a drag).
        //   - The active device did not move off the clickable widget.
        const bool is_click = state_ != State_::kDragging &&
            ! moved_enough_for_drag_ && is_on_same_widget;

        // If the alarm is not running, process the click if it is one, and
        // always reset everything.
        if (is_click)
            ProcessClick_(cur_tracker_->GetActuator(), is_modified_mode);
        ResetClick_();
    }
}

bool MainHandler::Impl_::StartOrContinueDrag_(const Event &event) {
    ASSERT(cur_tracker_);
    ASSERT(state_ == State_::kActivated || state_ == State_::kDragging);

    // If a drag has not started, check for enough motion for a drag. Do this
    // even if the Widget is not draggable, since sufficient motion should
    // cancel a click on a ClickableWidget as well.
    if (state_ == State_::kActivated && ! moved_enough_for_drag_)
        moved_enough_for_drag_ = cur_tracker_->MovedEnoughForDrag(event);

    // If the active Widget is null or is not draggable, stop.
    if (! IsDraggableWidget_(active_widget_))
        return false;

    // See if this is the start of a new drag.
    const bool is_drag_start =
        state_ == State_::kActivated && moved_enough_for_drag_;

    if (is_drag_start || state_ == State_::kDragging) {
        state_ = State_::kDragging;
        KLOG('d', "MainHandler now kDragging");
        ProcessDrag_(event, is_drag_start, event.is_modified_mode);
        return true;
    }
    return false;
}

void MainHandler::Impl_::ProcessDrag_(const Event &event, bool is_start,
                                      bool is_modified_mode) {
    ASSERT(state_ == State_::kDragging);
    ASSERT(moved_enough_for_drag_);
    ASSERT(cur_tracker_);
    ASSERT(precision_store_);

    // Set common items in DragInfo.
    drag_info_.is_modified_mode = is_modified_mode || click_state_.count > 1;
    drag_info_.linear_precision  = precision_store_->GetLinearPrecision();
    drag_info_.angular_precision = precision_store_->GetAngularPrecision();

    // Let the tracker set up the rest.
    auto draggable = std::dynamic_pointer_cast<DraggableWidget>(active_widget_);
    ASSERT(draggable);
    ASSERT(! draggable->IsHovering());

    // If starting a new drag.
    if (is_start) {
        // These are set once and used throughout the drag.
        drag_info_.path_to_stage  = context_.path_to_stage;
        drag_info_.path_to_widget =
            SG::FindNodePathInScene(*context_.scene, *draggable);

        cur_tracker_->FillActivationDragInfo(drag_info_);

        draggable->StartDrag(drag_info_);
        KLOG('h', "MainHandler kDragging with " << draggable->GetDesc()
             << " (" << drag_info_.path_to_widget.ToString() << ")");
        if (event.device == Event::Device::kMouse) {
            KLOG('d', "Mouse drag start at " << event.position2D
                 << (event.is_modified_mode ? " (MOD)" : ""));
        }
    }

    // Starting or continuing a current drag operation. Use the current info.
    cur_tracker_->FillEventDragInfo(event, drag_info_);
    draggable->ContinueDrag(drag_info_);
    if (event.device == Event::Device::kMouse) {
        KLOG('d', "Mouse drag at " << event.position2D
             << (event.is_modified_mode ? " (MOD)" : ""));
    }
}

void MainHandler::Impl_::ProcessClick_(Actuator actuator,
                                       bool is_modified_mode) {
    ASSERT(actuator != Actuator::kNone);
    const auto &tracker = GetTracker_(actuator);

    ClickInfo info;
    const auto duration = UTime::Now().SecondsSince(start_time_);
    info.is_modified_mode = is_modified_mode || click_state_.count > 1;
    info.is_long_press     = duration > TK::kLongPressTime;

    tracker->FillClickInfo(info);

    clicked_.Notify(info);

    state_ = State_::kWaiting;
    KLOG('h', "MainHandler kWaiting after click on "
         << (info.widget ? info.widget->GetDesc() : info.hit.path.ToString()));
}

void MainHandler::Impl_::ResetClick_() {
    ASSERT(! click_state_.alarm.IsRunning());
    cur_tracker_.reset();
    click_state_.Reset();
    KLOG('h', "MainHandler Reset click count to " << click_state_.count);
}

// ----------------------------------------------------------------------------
// MainHandler functions.
// ----------------------------------------------------------------------------

MainHandler::MainHandler(bool is_vr_enabled) : impl_(new Impl_(is_vr_enabled)) {
}

MainHandler::~MainHandler() {
}

void MainHandler::SetPrecisionStore(
    const PrecisionStorePtr &precision_store) {
    impl_->SetPrecisionStore(precision_store);
}

void MainHandler::SetContext(const Context &context) {
    impl_->SetContext(context);
}

void MainHandler::SetTouchable(const ITouchablePtr &touchable) {
    impl_->SetTouchable(touchable);
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

Event::Device MainHandler::GetActiveDevice() const {
    return impl_->GetActiveDevice();
}

Event::Device MainHandler::GetLastActiveDevice() const {
    return impl_->GetLastActiveDevice();
}

bool MainHandler::IsWaiting() const {
    return impl_->IsWaiting();
}

void MainHandler::SetPathFilter(const PathFilter &filter) {
    impl_->SetPathFilter(filter);
}

void MainHandler::ProcessUpdate(bool is_modified_mode) {
    impl_->ProcessUpdate(is_modified_mode);
}

bool MainHandler::HandleEvent(const Event &event) {
    return impl_->HandleEvent(event);
}

void MainHandler::Reset() {
    impl_->Reset();
}
