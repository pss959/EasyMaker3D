#pragma once

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

    virtual WidgetPtr GetWidgetForEvent(const Event &event) override;
    virtual WidgetPtr GetCurrentWidget() const override {
        return touched_widget_;
    }
    virtual void SetActive(bool is_active) override;
    virtual bool MovedEnoughForDrag(const Event &event) override;
    virtual void FillActivationDragInfo(DragInfo &info) override;
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) override;
    virtual void FillClickInfo(ClickInfo &info) override;
    virtual void Reset() override;

  private:
    ControllerPtr controller_;       ///< Controller to track.
    SG::NodePath  controller_path_;  ///< Scene path to Controller.
    WidgetPtr     touched_widget_;   ///< From last call to GetWidgetForEvent().

    /// Returns true if the given event contains data for a touch with the
    /// correct controller.
    bool IsTouchEvent_(const Event &event) const;
};
