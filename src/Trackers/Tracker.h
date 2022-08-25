#pragma once

#include "Base/Event.h"
#include "Base/Memory.h"
#include "Enums/Actuator.h"
#include "Enums/Hand.h"
#include "Math/Types.h"
#include "SG/NodePath.h"

struct ClickInfo;
struct DragInfo;

DECL_SHARED_PTR(Controller);
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

    /// Returns the Event::Device the Tracker tracks.
    virtual Event::Device GetDevice() const = 0;

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

    /// Returns a time (in seconds) to wait for multiple button clicks.
    /// Different devices may not be as easy to click quickly.
    virtual float GetClickTimeout() const = 0;

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
    /// Maintains data for derived classes that track a Controller.
    class ControllerData {
      public:
        /// Sets up an instance for the given Hand from the given SceneContext.
        void Init(const SceneContext &context, Hand hand);

        /// Returns a pointer to the Controller.
        const ControllerPtr & GetControllerPtr() const { return controller_; }

        /// Returns the Controller.
        Controller & GetController() const;

        /// Returns the other Controller.
        Controller & GetOtherController() const;

        /// Converts a point from world coordinates into the Controller's
        /// object coordinates.
        Point3f ToControllerCoords(const Point3f &p) const;

      private:
        ControllerPtr controller_;
        ControllerPtr other_controller_;
        SG::NodePath  path_;
    };

    /// Returns the SceneContext.
    SceneContext & GetContext() const;

    /// Convenience that updates hovering when the current Widget changes.
    static void UpdateWidgetHovering(const WidgetPtr &old_widget,
                                     const WidgetPtr &new_widget);

    /// Returns a scale factor to apply to motion to determine whether it is
    /// sufficient to start a drag operation. The active Widget is passed in;
    /// if it is not null, is clickable and has any click observers, extra
    /// motion is required to start a drag, since very small movements should
    /// not interfere with a click.
    static float GetMotionScale(const WidgetPtr &widget);

  private:
    Actuator        actuator_;
    SceneContextPtr context_;
};
