#pragma once

#include "Math/Types.h"
#include "SG/Hit.h"
#include "SG/NodePath.h"
#include "Trackers/Tracker.h"

DECL_SHARED_PTR(Controller);

/// TouchTracker is a derived Tracker class that tracks VR controller touchs.
///
/// \ingroup Trackers
class TouchTracker : public Tracker {
  public:
    explicit TouchTracker(Actuator actuator);

    /// Redefines this to also store Controller info.
    virtual void SetSceneContext(const SceneContextPtr &context) override;

    // The TouchTracker never hovers any Widgets.
    virtual void UpdateHovering(const Event &event) override {}
    virtual void StopHovering() override {}

    virtual bool IsActivation(const Event &event, WidgetPtr &widget) override;
    virtual bool IsDeactivation(const Event &event, WidgetPtr &widget) override;
    virtual bool MovedEnoughForDrag(const Event &event) override;
    virtual void FillActivationDragInfo(DragInfo &info) override;
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) override;
    virtual void FillClickInfo(ClickInfo &info) override;
    virtual void Reset() override;

  private:
    ControllerPtr controller_;       ///< Controller to track.
    SG::NodePath  controller_path_;  ///< Scene path to Controller.
    Ray           activation_ray_;   ///< Ray for the Event at activation.
    SG::Hit       activation_hit_;   ///< Hit for the Event at activation.
    WidgetPtr     touched_widget_;   ///< From last call to GetWidgetForEvent().

    /// If the given event contains data for a touch with the correct
    /// controller, this sets the touch position and returns true.
    bool GetTouchPos_(const Event &event, Point3f &pos) const;
};
