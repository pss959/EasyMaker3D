#pragma once

#include "Commands/ChangePointTargetCommand.h"
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

    /// Returns the current point target.
    const PointTarget & GetPointTarget() const {
        return point_target_widget_->GetPointTarget();
    }

    /// Moves the point target to match the given one.
    void SetPointTarget(const PointTarget &target) {
        point_target_widget_->SetPointTarget(target);
    }

    /// Returns the current edge target. This will never be null.
    // XXXX
    // const EdgeTarget  & GetEdgeTarget()  const {
    //     return edge_target_widget_->GetEdgeTarget();
    // }

    /// \name Target Snapping Functions for Tools.
    /// These functions are provided for Tool classes that implement snapping
    /// to targets during drag operations.
    ///@{

    /// Any Tool that uses a snapping during a drag operation should call this
    /// to indicate the beginning of an interactive operation that involves
    /// snapping.
    void StartSnapping();

    /// Any Tool that uses a snapping during a drag operation should call this
    /// end of an interactive operation that involves snapping.
    void EndSnapping();

    /// Snaps motion to the point target if it is visible. Consider the plane
    /// perpendicular to the motion vector that passes through the target
    /// point. When the given current point (start_pos + motion_vec) is close
    /// enough to that plane, this updates motion_vec so that the resulting
    /// point is on the plane and returns true.  Otherwise, motion_vec is left
    /// alone and false is returned.  This also enables visual feedback for the
    /// PointTargetWidget. All values are in stage coordinates.
    bool SnapToPoint(const Point3f &start_pos, Vector3f &motion_vec);

    ///@}

  private:
    CommandManagerPtr command_manager_;

    PointTargetWidgetPtr point_target_widget_;

    ChangePointTargetCommandPtr point_command_;

    /// Notifies when a target is activated or deactivated.
    Util::Notifier<bool> target_activation_;

    /// \name Widget callbacks.
    void PointActivated_(bool is_activation);
    void PointChanged_();
};

typedef std::shared_ptr<TargetManager> TargetManagerPtr;
