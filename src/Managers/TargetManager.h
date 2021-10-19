#pragma once

#include "Managers/CommandManager.h"
#include "Util/Notifier.h"
#include "Widgets/PointTargetWidget.h"

/// The TargetManager class manages interactive targets that may be placed on
/// Models in the scene or the stage. There is a separate interface for each
/// type of target.
///
/// The Activation event delegate can be used to detect when a target widget is
/// actively being dragged.
///
/// NOTE: All snapping must take place in stage coordinates because snapping
/// can work across different Models.
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
    /// deactivated. It is passed a flag indicating whether it is an activation
    /// or deactivation.
    Util::Notifier<bool> & GetTargetActivation() { return target_activation_; }

  private:
    CommandManagerPtr command_manager_;

    PointTargetWidgetPtr point_target_widget_;

    /// Notifies when a target is activated or deactivated.
    Util::Notifier<bool> target_activation_;

    /// \name Widget callbacks.
    void PointActivated(bool is_activation);
    void PointMoved();
};

typedef std::shared_ptr<TargetManager> TargetManagerPtr;
