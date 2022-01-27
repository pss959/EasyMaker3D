﻿#pragma once

#include "Managers/CommandManager.h"
#include "Targets/EdgeTarget.h"
#include "Targets/PointTarget.h"
#include "Util/Notifier.h"
#include "Widgets/PointTargetWidget.h"

/// The TargetManager class manages interactive targets that may be placed on
/// Models in the scene or the Stage. There is a separate interface for each
/// type of target.
///
/// The Activation event delegate can be used to detect when a target widget is
/// actively being dragged.
///
/// NOTE: All snapping must take place in stage coordinates.
///
/// \ingroup Managers
class TargetManager {
  public:
    /// The constructor is passed the CommandManager that is used to issue
    /// commands to move or modify targets.
    explicit TargetManager(const CommandManagerPtr &command_manager);

    /// Initializes the point target using the given PointTargetWidget, which
    /// is assumed to be in the correct place in the scene graph.
    void InitPointTarget(const PointTargetWidgetPtr &widget);

    /// Returns a Notifier that is invoked when a target is activated or
    /// deactivated for a drag operation. It is passed a flag indicating
    /// whether it is an activation or deactivation.
    Util::Notifier<bool> & GetTargetActivation() { return target_activation_; }

    /// Returns true if the point target is currently visible.
    bool IsPointTargetVisible();

    /// Returns true if the edge target is currently visible.
    bool IsEdgeTargetVisible();

    /// Toggles the display of the point target. Returns the new state.
    bool TogglePointTarget();

    /// Toggles the display of the edge target. Returns the new state.
    bool ToggleEdgeTarget();

    /// Returns the current point target. This will never be null.
    const PointTargetPtr & GetPointTarget() const { return point_target_; }

    /// Returns the current edge target. This will never be null.
    const EdgeTargetPtr  & GetEdgeTarget()  const { return edge_target_; }

  private:
    CommandManagerPtr command_manager_;

    PointTargetWidgetPtr point_target_widget_;

    PointTargetPtr  point_target_;
    EdgeTargetPtr   edge_target_;

    /// Notifies when a target is activated or deactivated.
    Util::Notifier<bool> target_activation_;

    /// \name Widget callbacks.
    void PointActivated_(bool is_activation);
    void PointMoved_();
};

typedef std::shared_ptr<TargetManager> TargetManagerPtr;
