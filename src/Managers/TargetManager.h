#pragma once

#include "Dimensionality.h"
#include "Memory.h"
#include "SG/NodePath.h"
#include "Util/Notifier.h"

class TargetWidgetBase;
DECL_SHARED_PTR(ChangeEdgeTargetCommand);
DECL_SHARED_PTR(ChangePointTargetCommand);
DECL_SHARED_PTR(CommandManager);
DECL_SHARED_PTR(EdgeTarget);
DECL_SHARED_PTR(EdgeTargetWidget);
DECL_SHARED_PTR(PointTarget);
DECL_SHARED_PTR(PointTargetWidget);
DECL_SHARED_PTR(TargetManager);

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

    /// Sets the path to the Stage for coordinate conversions.
    void SetPathToStage(const SG::NodePath &path);

    /// Initializes the point and edge targets using the given
    /// PointTargetWidget and EdgeTargetWidget, which are assumed to be in the
    /// correct place in the scene graph.
    void InitTargets(const PointTargetWidgetPtr &ptw,
                     const EdgeTargetWidgetPtr &etw);

    /// Returns a Notifier that is invoked when a target is activated or
    /// deactivated for a drag operation. It is passed a flag indicating
    /// whether it is an activation or deactivation.
    Util::Notifier<bool> & GetTargetActivation() { return target_activation_; }

    /// Returns true if the point target is currently visible.
    bool IsPointTargetVisible();

    /// Returns true if the edge target is currently visible.
    bool IsEdgeTargetVisible();

    // Sets the visibility of the point target.
    void SetPointTargetVisible(bool visible);

    // Sets the visibility of the edge target.
    void SetEdgeTargetVisible(bool visible);

    /// Returns the current point target.
    const PointTarget & GetPointTarget() const;

    /// Moves the point target to match the given one.
    void SetPointTarget(const PointTarget &target);

    /// Returns the current edge target. This will never be null.
    const EdgeTarget & GetEdgeTarget() const;

    /// Moves the edge target to match the given one.
    void SetEdgeTarget(const EdgeTarget &target);

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

    /// Snaps the given direction vector (in stage coordinates) to the current
    /// point target direction, if it is visible. If the direction is close
    /// enough to the target, this sets rot to the rotation that rotates the
    /// direction to the target (or in the exact opposite direction) and
    /// returns true. Otherwise, it leaves rot alone.
    bool SnapToDirection(const Vector3f &dir, Rotationf &rot);

    /// If the edge target is visible, this returns true if the given
    /// stage-space length is close enough to the edge target length for
    /// snapping. It also enables visual feedback for the EdgeTargetWidget.
    bool SnapToLength(float length);

    /// If the edge target is visible, this checks each element in the given
    /// vector that is present in the given Dimensionality to see if it is
    /// close enough to the edge target length for snapping.  This returns a
    /// Dimensionality instance representing all snapped dimensions. This also
    /// enables visual feedback for the EdgeTargetWidget if snapping occurred.
    Dimensionality SnapToLength(const Dimensionality &dims,
                                const Vector3f &vec);

    /// If the two given unit direction vectors are close enough (by angle) for
    /// snapping to occur, this returns true and sets rot to the rotation to
    /// bring v0 to v1. Leaves rot alone if this returns false.
    static bool ShouldSnapDirections(const Vector3f &v0, const Vector3f &v1,
                                     Rotationf &rot);

    ///@}

  private:
    CommandManagerPtr           command_manager_;
    SG::NodePath                path_to_stage_;
    PointTargetWidgetPtr        point_target_widget_;
    EdgeTargetWidgetPtr         edge_target_widget_;
    ChangePointTargetCommandPtr point_command_;
    ChangeEdgeTargetCommandPtr  edge_command_;

    /// Notifies when a target is activated or deactivated.
    Util::Notifier<bool> target_activation_;

    /// \name Widget callbacks.
    ///@{
    void PointActivated_(bool is_activation);
    void PointChanged_();
    void EdgeActivated_(bool is_activation);
    void EdgeChanged_();
    void EdgeClicked_();
    ///@}

    /// Implements edge snapping, returning true if the given length is
    /// snapped. If so, it sets diff to the absolute value of the difference.
    bool SnapToLengthWithDiff_(float length, float &diff);

    /// Calls ShowSnapFeedback() for the given Widget.
    void ShowSnapFeedback_(TargetWidgetBase &widget, bool is_snapped);
};
