#pragma once

#include "Base/Event.h"
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
    virtual Event::Device GetDevice() const override;

    /// Sets the current Grippable info. If the grippable pointer is not null,
    /// the path will be from the scene root to the Grippable, allowing for
    /// coordinate conversions.
    void SetGrippable(const GrippablePtr &grippable, const SG::NodePath &path) {
        grippable_      = grippable;
        grippable_path_ = path;
    }

    virtual void UpdateHovering(const Event &event) override;
    virtual void StopHovering() override;
    virtual bool IsActivation(const Event &event, WidgetPtr &widget) override;
    virtual bool IsDeactivation(const Event &event, WidgetPtr &widget) override;
    virtual float GetClickTimeout() const override;
    virtual void FillActivationDragInfo(DragInfo &info) override;
    virtual bool MovedEnoughForDrag(const Event &event) override;
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) override;
    virtual void FillClickInfo(ClickInfo &info) override;
    virtual void Reset() override;

  protected:
    virtual bool IsLeft() const override {
        return GetActuator() == Actuator::kLeftGrip;
    }

  private:
    /// Stores activation or current grip data.
    struct Data_ {
        Grippable::GripInfo info;
        Point3f             position;
        Rotationf           orientation;
    };

    GrippablePtr  grippable_;        ///< Active Grippable (or null).
    SG::NodePath  grippable_path_;   ///< Path to active Grippable (or empty).
    Data_         current_data_;     ///< Grip data for current Event.
    Data_         activation_data_;  ///< Grip data for the Event at activation.
    WidgetPtr     current_widget_;   ///< Current tracked Widget (or null).

    /// If the given Event relates to this Tracker, this updates the current
    /// Data_, stores the intersected Widget, if any, in widget, and returns
    /// true. The \p add_info flag is passed to GetGripData_();
    bool UpdateCurrentData_(const Event &event, bool add_info,
                            WidgetPtr &widget);

    /// If the given event contains data for a grip with the correct
    /// controller, this fills in data and returns true. If add_info is true,
    /// this also fills in the GripInfo in the data.
    bool GetGripData_(const Event &event, bool add_info, Data_ &data) const;

    /// Updates the Controllers when the active state changes.
    void UpdateControllers_(bool is_active);
};
