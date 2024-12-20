//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Base/Event.h"
#include "Enums/Actuator.h"
#include "Enums/Hand.h"
#include "Math/Types.h"
#include "SG/NodePath.h"
#include "Util/Memory.h"

struct ClickInfo;
struct DragInfo;

DECL_SHARED_PTR(Controller);
DECL_SHARED_PTR(Tracker);
DECL_SHARED_PTR(Widget);
namespace SG { DECL_SHARED_PTR(Scene); }

/// Tracker is an abstract base class for actuator trackers used by the
/// MainHandler. A Tracker is responsible for responding to Event instances to
/// activate, update, and deactivate interactive Widgets. Derived classes are
/// implemented to hide details of each type of actuator and its corresponding
/// device.
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

    /// \name Initialization
    ///@{

    /// Sets the current SG::Scene and controllers.
    void Init(const SG::ScenePtr &scene,
              const ControllerPtr &lc, const ControllerPtr &rc);

    ///@}

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
    /// this sets \p widget to the activated Widget (if any), and returns true.
    virtual bool IsActivation(const Event &event, WidgetPtr &widget) = 0;

    /// This is called only if the Tracker has been activated. If the given
    /// Event represents a deactivation of the Tracker's actuator, this sets \p
    /// widget to the new current Widget (if any) and returns true.
    virtual bool IsDeactivation(const Event &event, WidgetPtr &widget) = 0;

    ///@}

    /// \name Clicking and Dragging
    ///@{

    /// Returns a time (in seconds) to wait for multiple button clicks.
    /// Different devices may not be as easy to click quickly.
    virtual float GetClickTimeout() const = 0;

    /// Sets up the pointer-based fields or grip-based fields in the given
    /// DragInfo from the activation data.
    virtual void FillActivationDragInfo(DragInfo &info) = 0;

    /// Returns true if the given Event represents enough motion to consider
    /// this a drag. This should be called only after activation.
    virtual bool MovedEnoughForDrag(const Event &event) = 0;

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
    /// Returns the current SG::Scene.
    const SG::ScenePtr & GetScene() const { return scene_; }

    /// \name Functions for Controller-based Trackers
    ///@{

    /// Returns true if this Tracker is for the left Controller. The base class
    /// defines this to assert, since it should be defined in all
    /// Controller-based classes.
    virtual bool IsLeft() const;

    /// Returns the Controller for this Tracker.
    const ControllerPtr & GetController() const {
        return IsLeft() ? left_controller_ : right_controller_;
    }

    /// Returns the other Controller.
    const ControllerPtr & GetOtherController() const {
        return IsLeft() ? right_controller_ : left_controller_;
    }

    /// Convenience that converts a point from world coordinates into the
    /// Controller's object coordinates.
    Point3f ToControllerCoords(const Point3f &p) const;

    ///@}

    /// Convenience that updates hovering when the current Widget changes.
    void UpdateWidgetHovering(const WidgetPtr &old_widget,
                              const WidgetPtr &new_widget);

    /// Returns a scale factor to apply to motion to determine whether it is
    /// sufficient to start a drag operation. The active Widget is passed in;
    /// if it is not null, is clickable and has any click observers, extra
    /// motion is required to start a drag, since very small movements should
    /// not interfere with a click.
    static float GetMotionScale(const WidgetPtr &widget);

  private:
    Actuator        actuator_;
    SG::ScenePtr    scene_;
    ControllerPtr   left_controller_;
    ControllerPtr   right_controller_;
    SG::NodePath    left_controller_path_;
    SG::NodePath    right_controller_path_;
};
