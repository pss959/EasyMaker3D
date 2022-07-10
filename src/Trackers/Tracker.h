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

    /// \name Widget Hovering
    ///@{

    /// Updates Widget hovering based on the given Event.
    virtual void UpdateHovering(const Event &event) = 0;

    /// Stops Widget hovering if there is any currently occurring.
    virtual void StopHovering() = 0;

    ///@}

    /// \name Activation and Deactivation
    ///@{

    /// If the given Event represents an activation of the Tracker's actuator,
    /// this processes the activation, sets widget to the activated Widget (if
    /// any), and returns true.
    virtual bool IsActivation(const Event &event, WidgetPtr &widget) = 0;

    /// This is called only if the Tracker has been activated. If the given
    /// Event represents a deactivation of the Tracker's actuator, this
    /// processes the deactivation, sets widget to the current Widget (if any),
    /// and returns true.
    virtual bool IsDeactivation(const Event &event, WidgetPtr &widget) = 0;

    ///@}

    /// \name Clicking and Dragging
    ///@{

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

    ///@}

    /// Resets all state.
    virtual void Reset() = 0;

  protected:
    /// Returns the SceneContext.
    SceneContext & GetContext() const;

    /// Convenience that updates hovering when the current Widget changes.
    static void UpdateWidgetHovering(const WidgetPtr &old_widget,
                                     const WidgetPtr &new_widget);

  private:
    Actuator        actuator_;
    SceneContextPtr context_;
};
