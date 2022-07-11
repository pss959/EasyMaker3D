#pragma once

#include "Base/Event.h"
#include "Math/Types.h"
#include "SG/NodePath.h"
#include "Trackers/Tracker.h"

DECL_SHARED_PTR(Controller);
DECL_SHARED_PTR(ITouchable);

/// TouchTracker is a derived Tracker class that tracks VR controller touchs.
///
/// \ingroup Trackers
class TouchTracker : public Tracker {
  public:
    explicit TouchTracker(Actuator actuator);

    /// Sets the current ITouchable. If not null, the ITouchable will be the
    /// target for any touch-based clicks or drags.
    void SetTouchable(const ITouchablePtr &touchable) {
        touchable_ = touchable;
    }

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
    ITouchablePtr touchable_;        ///< Active Touchable (or null).
    Point3f       activation_pos_;   ///< Touch position at activation.
    WidgetPtr     current_widget_;   ///< Current tracked Widget (or null).

    Event::Device GetDevice_() const;

    /// If the given event contains data for a touch with the correct
    /// controller, this sets the touch position and returns true.
    bool GetTouchPos_(const Event &event, Point3f &pos) const;
};
