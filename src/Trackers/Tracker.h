#pragma once

#include "Base/Memory.h"
#include "Enums/Actuator.h"

struct ClickInfo;
struct DragInfo;
struct Event;

DECL_SHARED_PTR(SceneContext);
DECL_SHARED_PTR(Tracker);
DECL_SHARED_PTR(Widget);

/// Tracker is an abstract base class for actuator trackers used by the
/// MainHandler. Derived classes are implemented to hide details of each type
/// of actuator and its corresponding device.
///
/// \ingroup Trackers
class Tracker {
  public:
    /// The constructor is passed the Actuator this tracker is for.
    explicit Tracker(Actuator actuator);

    /// Returns the Actuator this tracker is for.
    Actuator GetActuator() const { return actuator_; }

    /// Sets or updates the SceneContext to use.
    virtual void SetSceneContext(const SceneContextPtr &context);

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
    /// current state from the tracker.
    virtual void FillClickInfo(ClickInfo &info) = 0;

    /// Resets all state.
    virtual void Reset() = 0;

  protected:
    /// Returns the SceneContext.
    SceneContext & GetContext() const;

    /// Convenience that updates hovering when the current Widget changes.
    static void UpdateHover(const WidgetPtr &old_widget,
                            const WidgetPtr &new_widget);

  private:
    Actuator        actuator_;
    SceneContextPtr context_;
};
