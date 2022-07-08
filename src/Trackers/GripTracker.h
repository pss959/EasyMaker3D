#pragma once

#include "Items/Grippable.h"
#include "Math/Types.h"
#include "SG/NodePath.h"
#include "Trackers/Tracker.h"

DECL_SHARED_PTR(Controller);
DECL_SHARED_PTR(Grippable);

/// GripTracker is a derived Tracker class that tracks VR controller grips.
///
/// \ingroup Trackers
class GripTracker : public Tracker {
  public:
    explicit GripTracker(Actuator actuator);

    /// Sets the current Grippable info. If the grippable pointer is not null,
    /// the path will be from the scene root to the Grippable, allowing for
    /// coordinate conversions.
    void SetGrippable(const GrippablePtr &grippable, const SG::NodePath &path) {
        grippable_      = grippable;
        grippable_path_ = path;
    }

    /// Redefines this to also store Controller info.
    virtual void SetSceneContext(const SceneContextPtr &context) override;

    virtual WidgetPtr GetWidgetForEvent(const Event &event) override;
    virtual WidgetPtr GetCurrentWidget() const override {
        return hovered_widget_;
    }
    virtual void SetActive(bool is_active) override;
    virtual bool MovedEnoughForDrag(const Event &event) override;
    virtual void FillActivationDragInfo(DragInfo &info) override;
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) override;
    virtual void FillClickInfo(ClickInfo &info) override;
    virtual void Reset() override;

  private:
    /// Stores activation or current grip data.
    struct Data_ {
        Grippable::GripInfo info;
        Point3f             position;
        Rotationf           orientation;
    };

    ControllerPtr controller_;       ///< Controller to track.
    SG::NodePath  controller_path_;  ///< Scene path to Controller.

    GrippablePtr  grippable_;        ///< Active Grippable (or null).
    SG::NodePath  grippable_path_;   ///< Path to active Grippable (or empty).

    Data_         current_data_;     ///< Grip data for current Event.
    Data_         activation_data_;  ///< Grip data for the Event at activation.
    WidgetPtr     hovered_widget_;   ///< From last call to GetWidgetForEvent().

    /// Returns true if the given event contains data for a grip with the
    /// correct controller.
    bool IsGripEvent_(const Event &event) const;
};
